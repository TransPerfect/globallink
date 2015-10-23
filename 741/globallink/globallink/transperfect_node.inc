<?php

require_once TPT_ROOT . '/' . ('transperfect_settings.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_common.inc');

/**
 * Sends content to GlobalLink for translation.
 *
 * @param array $nids
 *   The array of node nids.
 * @param string $pd4
 *   The project director details.
 * @param string $submission_name
 *   The name of the submission.
 * @param string $due_date
 *   When the translation is due.
 * @param string $project_code
 *   The project's registered code.
 * @param string $source_locale
 *   The locale of the content being translated.
 * @param array $target_locale_arr
 *   Array of desired locales to translate into.
 * @param array $submission_details
 *   Associative array of details about the submission.
 *
 * @return
 *   Associative array representing a GlobalLink object.
 */
function transperfect_send_for_translations($nids, $pd4, $submission_name, $due_date, $project_code, $source_locale, $target_locale_arr, $submission_details) {
  $node_check = variable_get('transperfect_implementation_type', 0);
  $submitter = $submission_details['submitter'];
  $globallink_arr = array();
  foreach ($nids as $nid) {
    list($nid, $vid) = explode('-', $nid, 2);
    $rows = transperfect_get_sent_rows_by_nid($nid);
    $target_arr = $target_locale_arr;
    foreach ($rows as $row) {
      if (array_search($row->target, $target_locale_arr)) {
        unset($target_arr[$row->target]);
      }
    }

    if (empty($target_arr)) {
      transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_WARNING, "Target languages already sent. Skipping nid - [" . $nid . "]");
      continue;
    }

    $node = node_load($nid, $vid);

    if ($node_check == 1) {
      foreach ($target_arr as $key => $target_locale) {
        if (!transperfect_translate_node_for_language($node, transperfect_get_drupal_locale_code($target_locale))) {
          unset($target_arr[$key]);
        }
      }
    }

    if (empty($target_arr)) {
      transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_WARNING, "No target languages. Skipping nid - [" . $nid . "]");
      continue;
    }

    $drupal_target_arr = array();
    foreach ($target_arr as $target_locale) {
      array_push($drupal_target_arr, transperfect_get_drupal_locale_code($target_locale));
    }

    $tnid = NULL;
    $tvid = NULL;

    $name = '.xml';
    $xml = transperfect_get_xml($node, $drupal_target_arr, $tnid, $tvid, $name);
    if (!$xml) {
      transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_WARNING, "Cannot create XML. Skipping nid - [" . $nid . "]");
      continue;
    }

    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_DEBUG, $xml);

    $globallink = new GlobalLink();
    $globallink->nid = $node->nid;
    $globallink->vid = $node->vid;
    $globallink->title = $node->title;
    $globallink->type = $node->type;
    $globallink->metadata = 'Node';
    $globallink->sourceLocale = $source_locale;
    $globallink->targetLocale = $target_arr;
    $globallink->sourceXML = $xml;
    $globallink->sourceFileName = $name;
    $globallink->submissionName = $submission_name;
    $globallink->dueDate = $due_date;
    $globallink->submissionInstructions = $submission_details['instructions'] . "\nSubmitter: " . $submitter;
    $globallink_arr[] = $globallink;
  }

  if (!empty($globallink_arr)) {
    send_documents_for_translation_to_pd($globallink_arr, $pd4, $project_code, $submitter);
  }
  return $globallink_arr;
}

/**
 * Updates the ticket id for a specified node.
 *
 * @param array $arr
 *   An array of GlobalLink objects.
 * @param string $project_code
 *   The project's registered code.
 */
function transperfect_update_node_ticket_id($arr, $project_code) {
  foreach ($arr as $globallink) {
    $nid = $globallink->nid;
    $node = node_load($nid);
    $target_locale_arr = $globallink->targetLocale;
    foreach ($target_locale_arr as $target_locale) {
      $row = transperfect_get_row_by_nid_and_locale($nid, $globallink->sourceLocale, $target_locale);
      if ($row) {
        db_update('transperfect_core')
                ->fields(array('vid' => $globallink->vid, 'title' => $globallink->title, 'document_ticket' => $globallink->documentTicket,
                    'submission' => $globallink->submissionName, 'submission_ticket' => $globallink->submissionTicket,
                    'status' => TPT_STATUS_SENT_FOR_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'last_modified' => $node->changed, 'changed' => 0, 'project_code' => $project_code))
                ->condition('rid', $row->rid, '=')
                ->execute();
      }
      else {
        db_insert('transperfect_core')
                ->fields(array('nid' => $globallink->nid, 'vid' => $globallink->vid, 'type' => $globallink->type, 'title' => $globallink->title,
                    'source' => $globallink->sourceLocale, 'target' => $target_locale, 'document_ticket' => $globallink->documentTicket,
                    'submission' => $globallink->submissionName, 'submission_ticket' => $globallink->submissionTicket,
                    'status' => TPT_STATUS_SENT_FOR_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'last_modified' => $node->changed, 'changed' => 0, 'project_code' => $project_code))
                ->execute();
      }
    }
  }
}

/**
 * Updates node modified flag.
 *
 * @param array $nids
 *   Array of node nids.
 * @param string $source
 *   Transperfect locale code.
 * @param array $tgts
 *   Array of transperfect locale codes.
 */
function transperfect_update_node_change_flag($nids, $source, $tgts) {
  transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_INFO, "CHANGED Status Cleared for Node Ids - [" . implode(",", $nids) . "]");

  $insert_arr = array();
  $tgt_arr = array_keys($tgts);
  foreach ($tgt_arr as $tgt) {
    foreach ($nids as $nid) {
      $insert_arr[$tgt][$nid] = TRUE;
    }
  }
  foreach ($tgt_arr as $tgt) {
    foreach ($nids as $nid) {
      $result = db_select('transperfect_core', 'tc')
              ->fields('tc')
              ->condition('nid', $nid, '=')
              ->condition('target', $tgt, '=')
              ->execute();
      foreach ($result as $item) {
        db_update('transperfect_core')
                ->fields(array('timestamp' => REQUEST_TIME, 'changed' => 2))
                ->condition('rid', $item->rid, '=')
                ->execute();
        $insert_arr[$tgt][$nid] = FALSE;
      }
    }
  }

  foreach ($tgt_arr as $tgt) {
    foreach ($nids as $nid) {
      if (isset($insert_arr[$tgt]) && isset($insert_arr[$tgt][$nid])) {
        if ($insert_arr[$tgt][$nid]) {
          $node = node_load($nid);
          db_insert('transperfect_core')
                  ->fields(array('nid' => $nid, 'vid' => $node->vid, 'type' => $node->type, 'title' => $node->title,
                      'source' => $source, 'target' => $tgt, 'document_ticket' => '',
                      'submission' => '', 'submission_ticket' => '',
                      'status' => TPT_STATUS_PENDING_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'last_modified' => $node->changed, 'changed' => 2))
                  ->execute();
        }
      }
    }
  }
}

/**
 * Cancels select records.
 *
 * @param array $rowids
 *   Array of row ids to cancel select records for.
 * @param string $pd4
 *   The project director details.
 */
function transperfect_cancel_select_records($rowids, $pd4) {
  $globallink_arr = array();
  foreach ($rowids as $rid) {
    $row = transperfect_get_row($rid);
    $globallink = new GlobalLink();
    $globallink->tptRowId = $row->rid;
    $globallink->targetLocale = $row->target;
    $globallink->documentTicket = $row->document_ticket;
    $globallink->submissionTicket = $row->submission_ticket;
    $globallink_arr[$rid] = $globallink;
  }

  cancel_select_documents($pd4, $globallink_arr);
  transperfect_update_by_row_id($globallink_arr);
}

/**
 * Cancels submission to GlobalLink.
 *
 * @param string $selected_submission
 *   The submission ticket.
 */
function transperfect_cancel_submission($selected_submission) {
  $pd4 = transperfect_get_project_director_details();
  $globallink = new GlobalLink();
  $submission_name = transperfect_get_submission_name($selected_submission);
  $globallink->submissionName = $submission_name;
  $globallink->submissionTicket = $selected_submission;

  cancel_pd_submission($pd4, $globallink);
  transperfect_update_row_by_submission($globallink);
}

/**
 * Gets submission name from GlobalLink
 *
 * @param string $submission_ticket
 *   The submission ticket.
 *
 * @return string
 *   The name of the submission.
 */
function transperfect_get_submission_name($submission_ticket) {
  $query = db_select('transperfect_core', 'tc');
  $query->fields('tc');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $results = $query->execute();
  foreach ($results as $row) {
    if ($row->submission != '') {
      return $row->submission;
    }
  }
}

/**
 * Updates a row document on GlobalLink.
 *
 * @param array $globalLink_arr
 *   Array of GlobalLink objects.
 */
function transperfect_update_row_document(&$globalLink_arr) {
  foreach ($globalLink_arr as $globalLink) {
    if ($globalLink->cancelled) {
      db_update('transperfect_core')
              ->fields(array('status' => TPT_STATUS_PENDING_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'changed' => 1))
              ->condition('rid', $globallink->tptRowId, '=')
              ->execute();
    }
  }
}

/**
 * Updates a submission on GlobalLink.
 *
 * @param object $globalLink
 *   A GlobalLink object.
 */
function transperfect_update_row_submission(&$globalLink) {
  db_update('transperfect_core')
          ->fields(array('status' => TPT_STATUS_PENDING_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'changed' => 1))
          ->condition('submission_ticket', $globallink->submissionTicket, '=')
          ->condition('submission', $globallink->submissionName, '=')
          ->execute();
}

/**
 * Gets all active submission names.
 *
 * @return array
 *   Array containing all active submission names.
 */
function transperfect_get_distinct_active_submission_names() {
  $query = db_select('transperfect_core', 'tc');
  $query->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $query->distinct();
  $query->fields('tc');
  $results = $query->execute();
  $arr = array('' => '-- Select a Submission --');
  foreach ($results as $row) {
    $arr[$row->submission_ticket] = $row->submission;
  }
  return $arr;
}

/**
 * Gets submission name from nid.
 *
 * @param int $nid
 *   The node id to pass in.
 *
 * @return string
 *   The name of the submission.
 */
function transperfect_get_submission_from_nid($nid) {
  $query = db_select('transperfect_core', 'tc');
  $query->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $query->condition('nid', $nid, '=');
  $query->distinct();
  $query->fields('tc');
  $results = $query->execute();
  foreach ($results as $row) {
    // Return the first one
    return $row->submission;
  }
}

/**
 * Retrieves the row id.
 *
 * @param string $submission_ticket
 *   The translation submission ticket.
 * @param string $document_ticket
 *   The translation document ticket.
 * @param string $target_locale
 *   The locale the submission is being translated into.
 *
 * @return int
 *   The id of the row.
 */
function transperfect_get_tpt_row_id_from_submission($submission_ticket, $document_ticket, $target_locale) {
  $query = db_select('transperfect_core', 'tc');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $query->condition('document_ticket', $document_ticket, '=');
  $query->condition('target', $target_locale, '=');
  $query->fields('tc');
  $results = $query->execute();
  foreach ($results as $row) {
    //Return the first one
    return $row->rid;
  }
}

/**
 * Updates deleted records on GlobalLink.
 *
 * @param string $pd4
 *   The project director details.
 * @param object $globalLink
 *   A GlobalLink object.
 *
 * @return bool
 *   True if the update is successful.
 */
function transperfect_update_deleted_records($pd4, $globalLink) {
  try {
    $globallink->status = TPT_STATUS_SOURCE_DELETED;
    send_download_confirmation($globallink->targetTicket, $pd4);
    transperfect_update_status($globallink);
  }
  catch (SoapFault $se) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "SOAP Exception in Sending Download Confirmation - update_deleted_records - Code[$se->faultcode], Message[$se->faultstring]");
    form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
  }
  catch (Exception $e) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in update_deleted_records - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
    form_set_error('', check_plain('Error: ' . $e->getMessage()));
  }

  return TRUE;
}

/**
 * Gets translated content from GlobalLink.
 *
 * @param string $pd4
 *   The project director details.
 * @param array $globalLink_arr
 *   Array of GlobalLink objects.
 *
 * @return int
 *   Number of rows translated.
 */
function transperfect_get_translated_content($pd4, &$globalLink_arr) {
  try {
    $count = 0;
    foreach ($globallink_arr as $globallink) {
      if (!$globallink->sourceDeleted) {
        $globallink->targetXML = download_target_resource($pd4, $globallink->targetTicket);
        if (isset($globallink->targetXML)) {
          $count++;
          transperfect_update_node($globallink);
          if ($globallink->status != TPT_STATUS_ERROR) {
            send_download_confirmation($globallink->targetTicket, $pd4);
            transperfect_update_status($globallink);
          }
          else {
            $count--;
          }
        }
      }
    }
  }
  catch (SoapFault $se) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "SOAP Exception in Sending Download Confirmation - get_translated_content - Code[$se->faultcode], Message[$se->faultstring]");
    form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
  }
  catch (Exception $e) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in get_translated_content - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
    form_set_error('', check_plain('Error: ' . $e->getMessage()));
  }

  return $count;
}

/**
 * Updates the translation status.
 *
 * @param object $globalLink
 *   The GlobalLink translation data.
 */
function transperfect_update_tpt_status(&$globalLink) {
  switch ($globallink->status) {
    case TPT_STATUS_SOURCE_DELETED:
      $row = transperfect_get_row_by_nid_and_locale($globallink->nid, $globallink->sourceLocale, $globallink->targetLocale);

      db_update('transperfect_core')
      ->fields(array(
        'status' => TPT_STATUS_SOURCE_DELETED,
        'document_ticket' => '',
        'submission_ticket' => '',
        'timestamp' => REQUEST_TIME,
      ))
      ->condition('rid', $row->rid, '=')
      ->execute();

      break;
    case TPT_STATUS_ERROR:
      $row = transperfect_get_row_by_nid_and_locale($globallink->nid, $globallink->sourceLocale, $globallink->targetLocale);

      db_update('transperfect_core')
      ->fields(array(
        'status' => TPT_STATUS_ERROR,
        'timestamp' => REQUEST_TIME,
      ))
      ->condition('rid', $row->rid, '=')
      ->execute();

      break;
    default:
      $row = transperfect_get_row_by_nid_and_locale($globalLink->nid, $globalLink->sourceLocale, $globalLink->targetLocale);
      $node = node_load($row->nid);

      if ($node->vid != $row->vid) {
        db_update('transperfect_core')
        ->fields(array(
          'vid' => $node->vid,
          'title' => $node->title,
          'status' => TPT_STATUS_PENDING_TRANSLATIONS,
          'timestamp' => REQUEST_TIME,
        ))
        ->condition('rid', $row->rid, '=')
        ->execute();
      }
      else {
        db_update('transperfect_core')
        ->fields(array(
          'status' => TPT_STATUS_PENDING_TRANSLATIONS,
          'timestamp' => REQUEST_TIME,
        ))
        ->condition('rid', $row->rid, '=')
        ->execute();
      }
  }
}

/**
 * Updates node.
 *
 * @param object $globalLink
 *   A GlobalLink object.
 * @param array $t_arr
 *   The translated array. Defaults to null.
 */
function transperfect_update_node(&$globalLink, $t_arr = NULL) {
  $success = FALSE;
  try {
    $default_language = language_default();
    $default_language_code = $default_language->language;
    $target_locale_code = transperfect_get_drupal_locale_code($globallink->targetLocale);
    $publish_node = variable_get('transperfect_publish_node', 0);

    if ($t_arr != NULL) {
      $translated_arr = $t_arr;
    }
    else {
      $translated_arr = transperfect_get_translated_array($globallink->targetXML);
    }

    $target_nid = $translated_arr['nid'];
    $target_vid = $translated_arr['vid'];
    $target_title = '';
    if (isset($translated_arr['title'])) {
      $target_title = $translated_arr['title'];
    }

    $globallink->nid = $target_nid;
    $globallink->vid = $target_vid;

    $node = node_load($target_nid, $target_vid);

    if (!$node || is_null($node) || !is_object($node)) {
      $globallink->status = TPT_STATUS_SOURCE_DELETED;
      return;
    }

    // check if already a translation set is present
    $tnid = $node->nid;
    if (!empty($node->tnid) && $node->nid != $node->tnid) {
      $tnid = $node->tnid;
    }

    $node_arr = translation_node_get_translations($tnid);
    $update_flag = FALSE;
    if (sizeof($node_arr) > 0) {
      foreach ($node_arr as $_tnode) {
        if ($_tnode->language == $target_locale_code) {
          $update_flag = TRUE;
          break;
        }
      }
    }

    if ($update_flag) {
      foreach ($node_arr as $_tnode) {
        if ($node->nid == $_tnode->nid) {
          continue;
        }
        if ($_tnode->language != $target_locale_code) {
          continue;
        }

        // Create a node and save
        $target_node = node_load($_tnode->nid);
        $new_node = unserialize(serialize($node));
        unset($new_node->nid);
        unset($new_node->vid);

        $new_node->nid = $_tnode->nid;
        $new_node->vid = 0;
        $new_node->tnid = $node->tnid;
        $new_node->language = $_tnode->language;
        $new_node->revision = 1;

        module_load_include('inc', 'node', 'node.pages');
        // Fix for menu link issue
        // node_object_prepare($new_node);

        $nodepath = drupal_lookup_path('alias', "node/" . $node->nid);
        // Get the path object of the tnode, so that we can get the pid
        $path = path_load("node/" . $_tnode->nid);
        if ($nodepath === FALSE) {
          // Just leave empty for now
        }
        else {
          if ($path) {
            if (!empty($translated_arr['path'])) {
              $new_node->path = array('pid' => $path['pid'], 'alias' => $translated_arr['path'], 'pathauto' => FALSE);
            }
            else {
              $new_node->path = array('pid' => $path['pid'], 'alias' => $nodepath, 'pathauto' => FALSE);
            }
          }
          else {
            // update the path object with the alias
            $new_node->path = array('alias' => $nodepath, 'pathauto' => FALSE);
            if (!empty($translated_arr['path'])) {
              $new_node->path = array('alias' => $translated_arr['path'], 'pathauto' => FALSE);
            }
            else {
              $new_node->path = array('alias' => $nodepath, 'pathauto' => FALSE);
            }
          }
        }

        if ($publish_node == 0) {
          $new_node->status = 0;
        }
        elseif ($publish_node == 1) {
          $new_node->status = 1;
        }
        else {
          $new_node->status = $node->status;
        }

        if (module_exists('revisioning')) {
          $new_node->is_pending = FALSE;
          $new_node->revision_moderation = FALSE;
        }

        // update it with translated content
        if ($target_title != '') {
          $new_node->title = $target_title;
        }

        $new_node->tpt_skip = TRUE;

        if (module_exists('metatag')) {
          if (isset($translated_arr['metatag'])) {
            $target_metatag_arr = $translated_arr['metatag'];
            if (!empty($node->metatags[$node->language])) {
              $metatags_names = array_keys($node->metatags[$node->language]);
              $n_metatag = &$new_node->metatags[$new_node->language];
              foreach ($metatags_names as $name) {
                if (isset($target_metatag_arr[$name]) && isset($target_metatag_arr[$name][0])) {
                  $gl_obj = $target_metatag_arr[$name]['0'];
                  if (is_object($gl_obj)) {
                    $translated_content = $gl_obj->translatedContent;
                  }
                  else {
                    $translated_content = $gl_obj;
                  }
                  $n_metatag[$name] = array('value' => $translated_content);
                }
              }
            }
          }
        }

        if (module_exists('field_collection') && isset($translated_arr['field_collection'])) {
          // Unset the field collection as it will be created later
          $t_fc_arr = $translated_arr['field_collection'];
          $fcs = array_keys($t_fc_arr);
          foreach ($fcs as $fc) {
            if (isset($new_node->$fc)) {
              unset($new_node->$fc);
            }
          }
        }

        // Workbench Moderation Support
        $enabled = transperfect_content_type_workbench_enabled($new_node->type);

        if ($enabled) {
          $current_value = variable_get('transperfect_moderation_' . $new_node->type, FALSE);
          if ($current_value) {
            $new_node->workbench_moderation['current']->from_state = $current_value;
            $new_node->workbench_moderation['current']->state = $current_value;
            $new_node->workbench_moderation_state_current = $current_value;
            $new_node->workbench_moderation_state_new = $current_value;

            if ($current_value != workbench_moderation_state_published()) {
              $new_node->workbench_moderation['current']->published = 0;
              unset($new_node->workbench_moderation['published']);
            }
          }
        }

        transperfect_update_non_translatable_field($target_node, $new_node);
        $success = transperfect_save_translated_node_with_fields($node, $new_node, $translated_arr);

        // Exclude Title Module Support
        if ($success && module_exists('exclude_node_title')) {
          $exclude = _exclude_node_title($node->nid);
          $exclude_list = variable_get('exclude_node_title_nid_list', array());
          $is_excluded = array_search($new_node->nid, $exclude_list);
          if ($exclude && $is_excluded === FALSE) {
            $exclude_list[] = $new_node->nid;
            variable_set('exclude_node_title_nid_list', $exclude_list);
          }
          elseif ($exclude === FALSE && $is_excluded) {
            unset($exclude_list[$is_excluded]);
            variable_set('exclude_node_title_nid_list', $exclude_list);
          }
        }

        if ($success) {
          transperfect_update_node_tnid($node->nid, $tnid);
          $globallink->status = TPT_STATUS_PUBLISHED;
        }
        else {
          $globallink->status = TPT_STATUS_ERROR;
        }
      }
    }
    else {
      // Create a node and save
      $new_node = unserialize(serialize($node));
      unset($new_node->nid);
      unset($new_node->vid);

      module_load_include('inc', 'node', 'node.pages');
      node_object_prepare($new_node);

      // IF translating from default to any other language e.g. en to fr
      if ($default_language_code == $node->language) {
        // Set the tnid from the source node
        $new_node->tnid = $node->nid;
      }
      else {
        // Translating from non default to non default
        // Set the tnid from the default node
        $new_node->tnid = $tnid;
      }

      $nodepath = drupal_lookup_path('alias', "node/" . $node->nid);

      if ($nodepath === FALSE) {
        // Just leave empty for now
      }
      else {
        if (!empty($translated_arr['path'])) {
          $new_node->path = array('alias' => $translated_arr['path']);
        }
        else {
          $new_node->path = array('alias' => $nodepath);
        }
      }

      if ($publish_node == 0) {
        $new_node->status = 0;
      }
      elseif ($publish_node == 1) {
        $new_node->status = 1;
      }
      else {
        $new_node->status = $node->status;
      }

      if (module_exists('revisioning')) {
        $new_node->is_pending = FALSE;
        $new_node->revision_moderation = FALSE;
      }

      $lang = transperfect_get_drupal_locale_code($globallink->targetLocale);
      if ($lang == '') {
        throw new Exception("Locale Mapping not found.");
      }
      $new_node->language = $lang;

      // update it with translated content
      if ($target_title != '') {
        $new_node->title = $target_title;
      }
      $new_node->tpt_skip = TRUE;

      if (module_exists('metatag')) {
        if (isset($translated_arr['metatag'])) {
          $target_metatag_arr = $translated_arr['metatag'];
          $metatags_names = array_keys($node->metatags[$node->language]);
          $n_metatag = &$new_node->metatags[$new_node->language];
          foreach ($metatags_names as $name) {
            if (isset($target_metatag_arr[$name]) && isset($target_metatag_arr[$name][0])) {
              $gl_obj = $target_metatag_arr[$name]['0'];
              if (is_object($gl_obj)) {
                $translated_content = $gl_obj->translatedContent;
              }
              else {
                $translated_content = $gl_obj;
              }
              $n_metatag[$name] = array('value' => $translated_content);
            }
          }
        }
      }

      if (module_exists('field_collection') && isset($translated_arr['field_collection'])) {
        // Unset the field collection as it will be created later
        $t_fc_arr = $translated_arr['field_collection'];
        $fcs = array_keys($t_fc_arr);
        foreach ($fcs as $fc) {
          if (isset($new_node->$fc)) {
            unset($new_node->$fc);
          }
        }
      }

      // Workbench Moderation Support
      $enabled = transperfect_content_type_workbench_enabled($new_node->type);

      if ($enabled) {
        $current_value = variable_get('transperfect_moderation_' . $new_node->type, FALSE);
        if ($current_value) {
          $new_node->workbench_moderation['current']->from_state = $current_value;
          $new_node->workbench_moderation['current']->state = $current_value;
          $new_node->workbench_moderation_state_current = $current_value;
          $new_node->workbench_moderation_state_new = $current_value;

          if ($current_value != workbench_moderation_state_published()) {
            $new_node->workbench_moderation['current']->published = 0;
            unset($new_node->workbench_moderation['published']);
          }
        }
      }

      $success = transperfect_save_translated_node_with_fields($node, $new_node, $translated_arr);

      if ($success) {
         // Exclude Title Module Support
        if (module_exists('exclude_node_title')) {
          $exclude = _exclude_node_title($node->nid);
          if ($exclude) {
            $exclude_list = variable_get('exclude_node_title_nid_list', array());
            $exclude_list[] = $new_node->nid;
            variable_set('exclude_node_title_nid_list', $exclude_list);
          }
        }

        // Pathauto Module Support
        if ($nodepath) {
          if (module_exists('pathauto')) {
            $node->path['pathauto'] = FALSE;
          }
        }

        transperfect_update_node_tnid($node->nid, $tnid);
        $globallink->status = TPT_STATUS_PUBLISHED;
      }
      else {
        $globallink->status = TPT_STATUS_ERROR;
      }
    }
  }
  catch (Exception $e) {
    $globallink->status = TPT_STATUS_ERROR;
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in Receive Translations - Update Node - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
  }
}

/**
 * Gets translated array from XML input.
 *
 * @param string $xml
 *   A string of raw XML.
 *
 * @return array
 *   The array of translated content.
 */
function transperfect_get_translated_array($xml) {
  if (is_null($xml) || !is_string($xml) || $xml == '') {
    return array();
  }

  $dom = new DomDocument;
  $dom->preserveWhiteSpace = FALSE;
  $dom->loadXML($xml);

  $arr = array();

  $contents = $dom->getElementsByTagName('content');
  foreach ($contents as $content) {
    if (!is_null($content->attributes)) {
      foreach ($content->attributes as $attr_name => $attr_node) {
        switch ($attr_name) {
          case 'rid':
            $arr['rid'] = $attr_node->value;

            break;
          case 'nid':
            $arr['nid'] = $attr_node->value;

            break;
          case 'vid':
            $arr['vid'] = $attr_node->value;

            break;
        }
      }
    }
  }

  $titles = $dom->getElementsByTagName('title');
  foreach ($titles as $title) {
    $arr['title'] = $title->nodeValue;
  }

  $paths = $dom->getElementsByTagName('path');
  foreach ($paths as $path) {
    $arr['path'] = $path->nodeValue;
  }

  $field_image = $dom->getElementsByTagName('field_image');
  foreach ($field_image as $attr) {
    $field_image_obj = new GLFieldImage();
    if (!is_null($attr->attributes)) {
      foreach ($attr->attributes as $attr_name => $attr_node) {
        switch ($attr_name) {
          case 'type':
            if ($attr_node->value == 'title') {
              $field_image_obj->title = $attr->nodeValue;
            }
            elseif ($attr_node->value == 'alt') {
              $field_image_obj->alt = $attr->nodeValue;
            }
            continue 2;
          case 'delta':
            $field_image_obj->delta = $attr_node->value;
            continue 2;
          case 'field_name':
            $field_image_obj->field_name = $attr_node->value;
            continue 2;
        }
      }
      if (is_null($field_image_obj->delta)) {
        $field_image_obj->delta = '0';
      }
      if (isset($field_image_obj->field_name)) {
        if (isset($field_image_obj->title)) {
          $arr[$field_image_obj->field_name][LANGUAGE_NONE][$field_image_obj->delta]->title = $field_image_obj->title;
        }
        if (isset($field_image_obj->alt)) {
          $arr[$field_image_obj->field_name][LANGUAGE_NONE][$field_image_obj->delta]->alt = $field_image_obj->alt;
        }
      }
      else {
        $arr[$field_image_obj->field_name][LANGUAGE_NONE][$field_image_obj->delta] = $field_image_obj;
      }
    }
  }

  $fields = $dom->getElementsByTagName('field');
  foreach ($fields as $field) {
    $field_obj = new GLField();
    $field_obj->type = 'field';
    $field_obj->translatedContent = $field->nodeValue;

    if (!is_null($field->attributes)) {
      foreach ($field->attributes as $attr_name => $attr_node) {
        switch ($attr_name) {
          case 'entity_type':
            $field_obj->entityType = $attr_node->value;
            continue 2;
          case 'content_type':
            $field_obj->contentType = $attr_node->value;
            continue 2;
          case 'parent_fc':
            $field_obj->parentFCName = $attr_node->value;
            continue 2;
          case 'bundle':
            $field_obj->bundle = $attr_node->value;
            continue 2;
          case 'entity_id':
            $field_obj->entityId = $attr_node->value;
            continue 2;
          case 'field_name':
            $field_obj->fieldName = $attr_node->value;
            continue 2;
          case 'label':
            $field_obj->fieldLabel = $attr_node->value;
            continue 2;
          case 'delta':
            $field_obj->delta = $attr_node->value;
            continue 2;
          case 'format':
            $field_obj->format = $attr_node->value;
            continue 2;
          case 'langcode':
            $field_obj->langcode = $attr_node->value;
            continue 2;
        }
      }
      if (is_null($field_obj->entityId)) {
        $field_obj->entityId = '0';
      }
      if (is_null($field_obj->bundle)) {
        $field_obj->bundle = $field_obj->fieldName;
      }
      if (is_null($field_obj->langcode)) {
        $field_obj->langcode = LANGUAGE_NONE;
      }
      if (is_null($field_obj->delta)) {
        $field_obj->delta = '0';
      }
      if ($field_obj->entityType == 'node') {
        $arr[$field_obj->fieldName][$field_obj->langcode][$field_obj->delta] = $field_obj;
      }
      else {
        $arr['field_collection'][$field_obj->parentFCName][$field_obj->bundle][$field_obj->entityId][$field_obj->fieldName][$field_obj->langcode][$field_obj->delta] = $field_obj;
      }
    }
  }

  $metatags = $dom->getElementsByTagName('metatag');

  foreach ($metatags as $metatag) {

    $metatag_obj = new GLField();
    $metatag_obj->type = 'metatag';
    $metatag_obj->translatedContent = $metatag->nodeValue;

    if (!is_null($metatag->attributes)) {
      foreach ($metatag->attributes as $attr_name => $attr_node) {
        switch ($attr_name) {
          case 'entity_type':
            $metatag_obj->entityType = $attr_node->value;
            continue 2;
          case 'content_type':
            $field_obj->contentType = $attr_node->value;
            continue 2;
          case 'bundle':
            $field_obj->bundle = $attr_node->value;
            continue 2;
          case 'entity_id':
            $metatag_obj->entityId = $attr_node->value;
            continue 2;
          case 'name':
            $metatag_obj->fieldName = $attr_node->value;
            continue 2;
          case 'label':
            $metatag_obj->fieldLabel = $attr_node->value;
            continue 2;
        }
      }
      if (is_null($metatag_obj->entityId)) {
        $metatag_obj->entityId = '0';
      }
      if (is_null($metatag_obj->bundle)) {
        $metatag_obj->bundle = $metatag_obj->fieldName;
      }
      $arr['metatag'][$metatag_obj->bundle][$metatag_obj->entityId] = $metatag_obj;
    }
  }
  return $arr;
}

/**
 * Gets active submission from nid.
 *
 * @param int $nid
 *   The node id to pass in.
 *
 * @return array
 *   An array representing the active submission.
 */
function transperfect_get_active_submission_by_nid($nid) {

  $query = db_select('transperfect_core', 'tc');
  $query->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $query->condition('nid', $nid, '=');
  $query->fields('tc');
  $results = $query->execute();
  $arr = array();
  foreach ($results as $row) {
    if (array_key_exists($row->submission, $arr)) {
      $t_arr = $arr[$row->submission];
      $t_arr[$row->target] = $row->vid;
      $arr[$row->submission] = $t_arr;
    }
    else {
      $_arr = array($row->target => $row->vid);
      $arr[$row->submission] = $_arr;
    }
  }
  return $arr;
}

/**
 * Gets active submission rows from nid.
 *
 * @param int $nids
 *   The node id to pass in.
 *
 * @return array
 *   The array of the active submission.
 */
function transperfect_get_active_submission_rows_by_nid($nids) {
  $query = db_select('transperfect_core', 'tc');
  $query->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $query->condition('nid', $nids, 'IN');
  $query->fields('tc');
  $results = $query->execute();
  $arr = array();
  foreach ($results as $row) {
    if (array_key_exists($row->nid, $arr)) {
      array_push($arr[$row->nid], $row);
    }
    else {
      $arr[$row->nid] = array($row);
    }
  }
  $final_arr = array();
  foreach ($arr as $nid => $nid_arr) {
    $sub_arr = array();
    foreach ($nid_arr as $r) {
      if (array_key_exists($r->submission, $sub_arr)) {
        array_push($sub_arr[$r->submission], $r->target);
      }
      else {
        $sub_arr[$r->submission] = array($r->vid => $r->target);
      }
    }
    if (count($sub_arr) > 0) {
      $final_arr[$nid] = $sub_arr;
    }
  }
  if (count($final_arr) > 0) {
    return $final_arr;
  }
  return FALSE;
}

/**
 * Saves Transperfect node.
 *
 * @param object $node
 *   The node to save.
 *
 * @return
 *   True if save is successful.
 */
function transperfect_node_save(&$node) {
  try {
    node_save($node);
    return TRUE;
  }
  catch (Exception $e) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Error in creating a translated node - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
    return FALSE;
  }
}

/**
 * Saves translated node with fields.
 *
 * @param object $node
 *   The original node.
 * @param object $tnode
 *   The translated node.
 * @param array $t_arr
 *   The translated array.
 *
 * @return
 *   True if save is successful.
 */
function transperfect_save_translated_node_with_fields($node, &$tnode, $t_arr) {
  $field_arr = field_info_instances('node', $node->type);
  $keys = array_keys($field_arr);
  $non_translate_field_arr = transperfect_get_non_translatable_config_fields($node->type);
  foreach ($keys as $field) {
    $items = field_get_items('node', $node, $field);
    if ($items && !array_key_exists($field, $non_translate_field_arr)) {
      $field_def = field_read_field($field);
      if ($field_def['type'] != 'list_boolean' && $field_def['type'] != 'file' && $field_def['type'] != 'taxonomy_term_reference' && $field_def['type'] != 'field_collection') {
        if (isset($t_arr[$field])) {
          $format = '';
          $t_field_lang = LANGUAGE_NONE;
          $t_target_lang = LANGUAGE_NONE;
          if (key($t_arr[$field]) !== LANGUAGE_NONE) {
            $t_field_lang = key($t_arr[$field]);
            $t_target_lang = $tnode->language;
          }

          $t_field_arr = $t_arr[$field][$t_field_lang];
          $arr = $tnode->$field;
          if (empty($arr[$t_target_lang]) && $t_target_lang != LANGUAGE_NONE) {
            $arr[$t_target_lang] = $arr[$t_field_lang];
          }

          $updated = FALSE;
          foreach ($arr[$t_target_lang] as $delta => $n_field) {
            if (isset($t_field_arr[$delta])) {
              $f_arr = array();
              $gl_obj = $t_field_arr[$delta];
              if ($field_def['type'] == 'image') {
                if (is_object($gl_obj)) {
                  if (isset($gl_obj->title)) {
                    $n_field['title'] = $gl_obj->title;
                    $arr[$t_target_lang][$delta] = $n_field;
                  }
                  if (isset($gl_obj->alt)) {
                    $n_field['alt'] = $gl_obj->alt;
                    $arr[$t_target_lang][$delta] = $n_field;
                  }
                }
              }
              else {
                if (is_object($gl_obj)) {
                  $translated_content = $gl_obj->translatedContent;
                }
                else {
                  $translated_content = $gl_obj;
                }
                if (isset($n_field['format'])) {
                  $format = $n_field['format'];
                }
                if ($format != '') {
                  $f_arr['format'] = $format;
                }
                if ($field_def['type'] == 'link_field') {
                  $n_field['title'] = $translated_content;
                  $arr[$t_target_lang][$delta] = $n_field;
                }
                else {
                  $f_arr['value'] = $translated_content;
                  $arr[$t_target_lang][$delta] = $f_arr;
                }
              }
              $updated = TRUE;
            }
          }
          if ($updated) {
            $tnode->$field = $arr;
          }
        }
      }
    }
  }

  $is_hook_enabled = variable_get('transperfect_implementation_type', 0);
  if ($is_hook_enabled == 1) {
    transperfect_update_node_hook($node, $tnode);
  }

  $success = transperfect_node_save($tnode);
  if ($success) {
    if (module_exists("field_collection") && isset($t_arr['field_collection'])) {
      transperfect_save_field_collections($node, $tnode, $t_arr);
    }
  }

  return $success;
}

/**
 * Saves field collections.
 *
 * @param object $node
 *   The original node.
 * @param object $tnode
 *   The translated node.
 * @param array $t_arr
 *   The translated array.
 */
function transperfect_save_field_collections($node, $tnode, $t_arr) {
  $field_arr = field_info_instances('node', $node->type);
  $keys = array_keys($field_arr);
  foreach ($keys as $field) {
    $items = field_get_items('node', $node, $field);
    if ($items) {
      $field_def = field_read_field($field);
      if ($field_def['type'] == 'field_collection') {
        if (isset($t_arr['field_collection'][$field])) {
          foreach ($items as $entity_id_arr) {
            if (isset($entity_id_arr['value'])) {
              $fc_entity_id = $entity_id_arr['value'];
              transperfect_save_field_collections_recursively('node', $tnode, $fc_entity_id, $t_arr['field_collection'][$field]);
            }
          }
        }
      }
    }
  }
}

/**
 * Saves field collections recursively.
 *
 * @param string $entity_type
 *   The type of entity.
 * @param string $host_entity
 *   The host entity.
 * @param string $fc_entity_id
 *   The id of the field collection.
 * @param array $translated_fc_arr
 *   The array of translated field collections.
 */
function transperfect_save_field_collections_recursively($entity_type, $host_entity, $fc_entity_id, $translated_fc_arr) {
  $field_collection_item_entity = entity_load_unchanged('field_collection_item', $fc_entity_id);
  if (!$field_collection_item_entity) {
    return;
  }
  $field_collection_name = $field_collection_item_entity->field_name;

  $field_collection_item_array = get_object_vars($field_collection_item_entity);
  $arr = array_keys($field_collection_item_array);
  $new_field_collection_arr = array('field_name' => $field_collection_name);
  foreach ($arr as $key) {
    // Check if this key exists; If true then read this.
    $fc_field_def = field_read_field($key);
    if ($fc_field_def && !empty($fc_field_def) && isset($fc_field_def['type'])) {
      if ($fc_field_def['type'] != 'list_boolean' && $fc_field_def['type'] != 'file' && $fc_field_def['type'] != 'taxonomy_term_reference' && $fc_field_def['type'] != 'field_collection') {
        if (isset($field_collection_item_array[$key]) && isset($field_collection_item_array[$key][LANGUAGE_NONE])) {
          $field_data_arr = $field_collection_item_array[$key][LANGUAGE_NONE];
          foreach ($field_data_arr as $delta => $field_data) {
            if (isset($translated_fc_arr[$field_collection_name][$fc_entity_id][$key][LANGUAGE_NONE][$delta])) {
              $gl_obj = $translated_fc_arr[$field_collection_name][$fc_entity_id][$key][LANGUAGE_NONE][$delta];
              if (is_object($gl_obj)) {
                $translated_content = $gl_obj->translatedContent;
              }
              else {
                $translated_content = $gl_obj;
              }
              if (isset($field_data['format'])) {
                $format = $field_data['format'];
                $new_field_collection_arr[$key][LANGUAGE_NONE][$delta] = array('value' => $translated_content, 'format' => $format);
              }
              else {
                $new_field_collection_arr[$key][LANGUAGE_NONE][$delta] = array('value' => $translated_content);
              }
            }
            else {
              $new_field_collection_arr[$key][LANGUAGE_NONE][$delta] = $field_data;
            }
          }
        }
      }
      elseif ($fc_field_def['type'] != 'field_collection') {
        if (isset($field_collection_item_array[$key]) && isset($field_collection_item_array[$key][LANGUAGE_NONE])) {
          $field_data_arr = $field_collection_item_array[$key][LANGUAGE_NONE];
          $new_field_collection_arr[$key][LANGUAGE_NONE] = $field_data_arr;
        }
      }
    }
  }

  if ($entity_type != 'node') {
    $host_entity->tpt_skip = TRUE;
  }
  $field_collection_item = entity_create('field_collection_item', $new_field_collection_arr); // Create new field collection item.
  $field_collection_item->setHostEntity($entity_type, $host_entity); // Attach it to the node.
  $field_collection_item->save(TRUE); // Save field-collection item

  field_attach_presave($entity_type, $host_entity);
  field_attach_update($entity_type, $host_entity);

  // Now set the child FC if any
  foreach ($arr as $key) {
    // Check if this key exists; If true then read this.
    $fc_field_def = field_read_field($key);
    if ($fc_field_def && !empty($fc_field_def) && isset($fc_field_def['type'])) {
      if ($fc_field_def['type'] == 'field_collection') {
        $items = field_get_items('field_collection_item', $field_collection_item_entity, $key);
        if ($items) {
          foreach ($items as $entity_id_arr) {
            if (isset($entity_id_arr['value'])) {
              $fc_entity_id = $entity_id_arr['value'];
              transperfect_save_field_collections_recursively('field_collection_item', $field_collection_item, $fc_entity_id, $translated_fc_arr);
            }
          }
        }
      }
    }
  }
}

/**
 * Retrieves translation XML data.
 *
 * @param object $node
 *   The node data.
 * @param string $target_arr
 *   Array of locales node is being translated into.
 * @param int $tnid
 *   The node id for existing translations. Defaults to null.
 * @param int $tvid
 *    The node vid for existing translations. Defaults to null.
 * @param string $name
 *   The xml document name. Defaults to empty string.
 * @param bool $for_display
 *   Indicate for display purposes only. Defaults to false.
 *
 * @return string
 *   The node deleted status or the translation XML data.
 */
function transperfect_get_xml($node, $target_arr, $tnid = NULL, $tvid = NULL, &$name = '', $for_display = FALSE) {
  if (is_null($node)) {
    return TPT_STATUS_SOURCE_DELETED;
  }
  elseif (!$node) {
    return TPT_STATUS_SOURCE_DELETED;
  }

  if ($node && is_object($node)) {
    if ($node->language != 'en') {
      $name = 'Node_' . $node->nid . '_Non_English' . $name;
    }
    else {
      $name = transperfect_format_file_name($node->title) . $name;
    }
    $xml = transperfect_generate_xml_document($node, $target_arr, $tnid, $tvid, $for_display);
    return $xml;
  }

  return TPT_STATUS_SOURCE_DELETED;
}

/**
 * Builds XML document with transltion data.
 *
 * @param object $node
 *   The node data.
 * @param string $target_arr
 *   Array of locales node is being translated into.
 * @param int $tnid
 *   The node id for existing translations.
 * @param int $tvid
 *    The node vid for existing translations.
 * @param string $name
 *   The xml document name.
 * @param bool $for_display
 *   Indicate for display purposes only.
 *
 * @return string
 *   The transltion XML data.
 */
function transperfect_generate_xml_document($node, $target_arr, $tnid = NULL, $tvid = NULL, $for_display = FALSE) {
  $is_hook_enabled = variable_get('transperfect_implementation_type', 0);

  try {
    $dom = new DOMDocument('1.0', 'UTF-8');
    $dom->formatOutput = TRUE;

    $root = $dom->createElement("content");
    $nid = $dom->createAttribute('nid');
    if ($tnid != NULL) {
      $nid->value = $tnid;
    }
    else {
      $nid->value = $node->nid;
    }
    $root->appendChild($nid);

    $vid = $dom->createAttribute('vid');
    if ($tvid != NULL) {
      $vid->value = $tvid;
    }
    else {
      $vid->value = $node->vid;
    }
    $root->appendChild($vid);

    $url = $dom->createAttribute('pageUrl');
    if (isset($node->path)) {
      $url->value = url($node->path['source'], array('absolute' => TRUE));
    }
    else {
      $url->value = url('node/' . $node->nid, array('absolute' => TRUE));
    }
    $root->appendChild($url);

    $dom->appendChild($root);

    if ($for_display) {
      transperfect_insert_child_element($dom, $root, "title", $node->title);
    }
    elseif (transperfect_is_field_configured_for_translation('node', $node->type, 'title', $node->type) && $is_hook_enabled == 0) {
      transperfect_insert_child_element($dom, $root, "title", $node->title);
    }
    elseif ($is_hook_enabled == 1) {
      if (transperfect_is_field_translatable($node, 'title', $target_arr)) {
        transperfect_insert_child_element($dom, $root, "title", $node->title);
      }
    }

    $path = path_load('node/' . $node->nid);
    if (!empty($path)) {
      transperfect_insert_child_element($dom, $root, "path", $path['alias']);
    }

    $field_arr = field_info_instances('node', $node->type);
    $keys = array_keys($field_arr);
    foreach ($keys as $field) {
      $field_def = field_read_field($field);
      $langcode = field_language('node', $node, $field, NULL);
      $items = field_get_items('node', $node, $field, $langcode);
      if ($items) {
        $parent_fc = '';
        if ($field_def['type'] == 'field_collection') {
          $parent_fc = $field;
          if (isset($items[0]['revision_id'])) {
            $field_collection_item_entity = entity_load('field_collection_item', array($items[0]['value']), array('revision_id' => $items[0]['revision_id']));
          }
        }
        transperfect_traverse_fields_and_field_collections('node', $node->type, $parent_fc, $node->type, $node->nid, $items, $field, $dom, $root, $node, $target_arr, $is_hook_enabled, $langcode);
      }
    }

    if (module_exists("metatag")) {
      if (isset($node->metatags)) {
        if ($for_display) {
          if (!empty($node->metatags[$node->language])) {
            $metatags = $node->metatags[$node->language]; // langage of the node to access the metatags
            foreach ($metatags as $name => $value) {
              if (isset($value['value'])) {
                transperfect_insert_child_element($dom, $root, "metatag", $value['value'], array('entity_type' => 'node', 'name' => $name, 'label' => 'Metatag - ' . ucwords($name)));
              }
            }
          }
        }
        elseif (transperfect_is_field_configured_for_translation('node', $node->type, 'metatags', $node->type)) {
          if (!empty($node->metatags[$node->language])) {
            $metatags = $node->metatags[$node->language]; // langage of the node to access the metatags
            foreach ($metatags as $name => $value) {
              if (isset($value['value'])) {
                transperfect_insert_child_element($dom, $root, "metatag", $value['value'], array('entity_type' => 'node', 'name' => $name, 'label' => 'Metatag - ' . ucwords($name)));
              }
            }
          }
        }
      }
    }
  }
  catch (Exception $e) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in creating XML - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
    throw $e;
  }

  $root_element = $dom->getElementsByTagName('content')->item(0);
  if (!$root_element->hasChildNodes()) {
    return FALSE;
  }

  return $dom->saveXML();
}

/**
 * Recursively add fields and field collections to translation XML document.
 *
 * @param string $entity_type
 *   The type of entity.
 * @param string $content_type
 *   The content type.
 * @param string $parent_fc
 *   The parent field collection.
 * @param string $bundle
 *   The field collection bundle.
 * @param string $entity_id
 *   The id of the entity.
 * @param array $items
 *   Array of content.
 * @param string $field
 *   The name of the field.
 * @param array $dom
 *   Array representation of the DOM.
 * @param array $root
 *   The root of the DOM.
 * @param object $source_node
 *   The source node.
 * @param array $target_arr
 *   The target array.
 * @param bool $is_hook_enabled
 *   Whether or not the hook is enabled.  Defaults to false.
 * @param string $langcode
 *   The language code.  Defaults to LANGUAGE_NONE.
 */
function transperfect_traverse_fields_and_field_collections($entity_type, $content_type, $parent_fc, $bundle, $entity_id, $items, $field, $dom, $root, $source_node, $target_arr, $is_hook_enabled = 0, $langcode = LANGUAGE_NONE) {
  if (!$items) {
    return;
  }
  $field_def = field_read_field($field);
  if ($field_def['type'] != 'list_boolean' && $field_def['type'] != 'file' && $field_def['type'] != 'taxonomy_term_reference') {
    if ($field_def['type'] != 'field_collection') {
      if ($is_hook_enabled == 0) {
        if (transperfect_is_field_configured_for_translation($entity_type, $bundle, $field, $content_type)) {
          // Regular Text Field, get the content directly from items array
          foreach ($items as $delta => $item) {
            if (isset($item['value']) && is_string($item['value'])) {
              $f_label = field_info_instance($entity_type, $field, $bundle);
              $f_value = $item['value'];
              $f_format = (isset($item['format']) && !is_null($item['format'])) ? $item['format'] : '';
              transperfect_insert_child_element($dom, $root, "field", $f_value, array('entity_type' => $entity_type, 'content_type' => $content_type,
                  'parent_fc' => $parent_fc, 'bundle' => $bundle, 'entity_id' => $entity_id, 'field_name' => $field,
                  'label' => $f_label['label'], 'delta' => $delta, 'format' => $f_format, 'langcode' => $langcode));
            }
            elseif ($field_def['type'] == 'link_field' && isset($item['title']) && is_string($item['title'])) {
              $f_label = field_info_instance($entity_type, $field, $bundle);
              $f_value = $item['title'];
              $f_format = (isset($item['format']) && !is_null($item['format'])) ? $item['format'] : '';
              transperfect_insert_child_element($dom, $root, "field", $f_value, array('entity_type' => $entity_type, 'content_type' => $content_type,
                  'parent_fc' => $parent_fc, 'bundle' => $bundle, 'entity_id' => $entity_id, 'field_name' => $field,
                  'label' => $f_label['label'], 'delta' => $delta, 'format' => $f_format, 'langcode' => $langcode));
            }
            elseif ($field_def['type'] == 'image') {
              $f_label = field_info_instance($entity_type, $field, $bundle);
              if (isset($item['alt'])) {
                $f_value = $item['alt'];
                transperfect_insert_child_element($dom, $root, "field_image", $f_value, array('field_name' => $field, 'type' => 'alt', 'delta' => $delta));
              }
              if (isset($item['title'])) {
                $f_value = $item['title'];
                transperfect_insert_child_element($dom, $root, "field_image", $f_value, array('field_name' => $field, 'type' => 'title', 'delta' => $delta));
              }
            }
          }
        }
      }
      else {
        if (transperfect_is_field_translatable($source_node, $field, $target_arr)) {
          // Regular Text Field, get the content directly from items array
          foreach ($items as $delta => $item) {
            if (isset($item['value']) && is_string($item['value'])) {
              $f_label = field_info_instance($entity_type, $field, $bundle);
              $f_value = $item['value'];
              $f_format = (isset($item['format']) && !is_null($item['format'])) ? $item['format'] : '';
              transperfect_insert_child_element($dom, $root, "field", $f_value, array('entity_type' => $entity_type, 'content_type' => $content_type,
                  'parent_fc' => $parent_fc, 'bundle' => $bundle, 'entity_id' => $entity_id, 'field_name' => $field,
                  'label' => $f_label['label'], 'delta' => $delta, 'format' => $f_format, 'langcode' => $langcode));
            }
            elseif ($field_def['type'] == 'link_field' && isset($item['title']) && is_string($item['title'])) {
              $f_label = field_info_instance($entity_type, $field, $bundle);
              $f_value = $item['title'];
              $f_format = (isset($item['format']) && !is_null($item['format'])) ? $item['format'] : '';
              transperfect_insert_child_element($dom, $root, "field", $f_value, array('entity_type' => $entity_type, 'content_type' => $content_type,
                  'parent_fc' => $parent_fc, 'bundle' => $bundle, 'entity_id' => $entity_id, 'field_name' => $field,
                  'label' => $f_label['label'], 'delta' => $delta, 'format' => $f_format, 'langcode' => $langcode));
            }
          }
        }
      }
    }
    elseif (module_exists("field_collection")) {
      // Field Collection field, read the entity id from item and load
      // Entity object and then do recursion for nested field collections.
      foreach ($items as $entity_id_arr) {
        if (isset($entity_id_arr['value'])) {
          $fc_entity_id = $entity_id_arr['value'];
          $field_collection_item_entity_arr = array();
          if (isset($entity_id_arr['revision_id'])) {
            $field_collection_item_entity_arr = entity_load('field_collection_item', array($fc_entity_id), array('revision_id' => $entity_id_arr['revision_id']));
          }
          else {
            $field_collection_item_entity_arr = entity_load('field_collection_item', array($fc_entity_id));
          }
          if (!$field_collection_item_entity_arr || !is_array($field_collection_item_entity_arr) || sizeof($field_collection_item_entity_arr) < 1) {
            continue;
          }
          $field_collection_item_entity = $field_collection_item_entity_arr[$fc_entity_id];
          $field_collection_name = $field_collection_item_entity->field_name;
          $field_collection_item_array = get_object_vars($field_collection_item_entity);
          $arr = array_keys($field_collection_item_array);
          foreach ($arr as $key) {
            // Check if this key exists; If true then read this.
            $fc_field_def = field_read_field($key);
            if ($fc_field_def && !empty($fc_field_def) && isset($fc_field_def['type'])) {
              if ($fc_field_def['type'] != 'list_boolean' && $fc_field_def['type'] != 'file' && $fc_field_def['type'] != 'taxonomy_term_reference') {
                $fc_item = field_get_items('field_collection_item', $field_collection_item_entity, $key);
                if ($fc_item) {
                  transperfect_traverse_fields_and_field_collections('field_collection_item', $content_type, $parent_fc, $field_collection_name, $fc_entity_id, $fc_item, $key, $dom, $root, $source_node, $target_arr, $is_hook_enabled);
                }
              }
            }
          }
        }
      }
    }
  }
}

/**
 * Gets node title.
 *
 * @param object $globalLink
 *   A GlobalLink object.
 *
 * @return string
 *   The title of the node.
 */
function transperfect_get_node_title(&$globalLink) {
  $result = db_select('transperfect_core', 'tc')
          ->fields('tc')
          ->condition('document_ticket', $globallink->documentTicket, '=')
          ->condition('submission_ticket', $globallink->submissionTicket, '=')
          ->execute();
  foreach ($result as $item) {
    $nid = $item->nid;
    $node = node_load($nid, $item->vid);
    $globallink->nid = $nid;
    $globallink->vid = $item->vid;
    $globallink->tptRowId = $item->rid;
    $globallink->title = $item->title;

    switch ($item->status) {
      case TPT_STATUS_SENT_FOR_TRANSLATIONS:
        $globallink->status = TPT_STATUS_COMPLETED;

        break;
      case TPT_STATUS_ERROR:
        $globallink->status = TPT_STATUS_ERROR;

        break;
    }

    if (!$node || is_null($node) || !is_object($node)) {
      return FALSE;
    }
    else {
      $title = l(transperfect_format_display_string($node->title), 'node/' . $item->nid);
      return $title;
    }
  }
  return FALSE;
}

/**
 * Updates submission status for a translation on GlobalLink.
 *
 * @param string $submission_ticket
 *   The submission ticket.
 * @param string $status
 *   The submission status.  Defaults to TPT_STATUS_CANCELLED.
 */
function transperfect_update_submission_status($submission_ticket, $status = TPT_STATUS_CANCELLED) {
  db_update('transperfect_core')
          ->fields(array(
              'status' => $status,
              'timestamp' => REQUEST_TIME
                  )
          )
          ->condition('submission_ticket', $submission_ticket, '=')
          ->execute();
}

/**
 * Gets submission status for a translation on GlobalLink.
 */
function transperfect_get_submission_status() {
  $query = db_select('transperfect_core', 'tc');
  $query->fields('tc', array('submission_ticket'));
  $query->distinct();
  $query->condition('status', TPT_STATUS_SENT_FOR_TRANSLATIONS, '=');
  $results = $query->execute();
  foreach ($results as $row) {
    if ($row->submission_ticket) {
      try {
        $pd4 = transperfect_get_project_director_details();
        $status = get_status($pd4, $row->submission_ticket);
        if (!$status || $status == PD_STATUS_CANCELLED) {
          transperfect_update_submission_status($row->submission_ticket);
        }
      }
      catch (SoapFault $se) {
        transperfect_update_submission_status($row->submission_ticket);
      }
      catch (Exception $ex) {
        transperfect_update_submission_status($row->submission_ticket);
      }
    }
  }
}

/**
 * Checks translation status.
 *
 * @param array $rids_arr
 *   Array of row ids.
 *
 * @return array
 *   An array of row ids.
 */
function transperfect_check_tpt_status($rids_arr) {
  $status = TRUE;
  $query = db_select('transperfect_core', 'tc')
          ->fields('tc', array('rid'))
          ->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $results = $query->execute();
  $rows = array();
  foreach ($results as $item) {
    $rows[$item->rid] = $item->rid;
  }

  foreach ($rids_arr as $val) {
    if (!in_array($val, $rows)) {
      unset($rids_arr[$val]);
      $status = FALSE;
    }
  }

  if (!$status) {
    drupal_set_message(t('Cannot cancel documents that have been cancelled in Globallink.'), 'warning', NULL);
  }

  return $rids_arr;
}

/**
 * Clear cancelled documents on GlobalLink.
 *
 * @return int
 *   Number of cancelled documents that have been cleared.
 */
function transperfect_clear_cancelled_documents() {
  $count = 0;
  $query = db_select('transperfect_core', 'tc')
          ->fields('tc', array('submission_ticket'))
          ->distinct()
          ->condition('status', TPT_STATUS_CANCELLED, '=');
  $results = $query->execute();
  foreach ($results as $item) {
    transperfect_update_submission_status($item->submission_ticket, TPT_STATUS_PENDING_TRANSLATIONS);
    $count++;
  }

  return $count;
}
