<?php

require_once TPT_ROOT . '/' . ('transperfect_settings.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_common.inc');

function send_entity_for_translations($nids, $pd4, $submission_name, $due_date, $project_code, $source_locale, $target_locale_arr, $submission_details) {
  $node_check = variable_get('transperfect_implementation_type', 0);
  $submitter = $submission_details['submitter'];
  $drupal_locale_code = transperfect_get_drupal_locale_code($source_locale);
  $globallink_arr = array();
  foreach ($nids as $nid) {
    list($nid, $vid) = explode('-', $nid, 2);
    $rows = get_sent_tpt_entity_rows_by_nid($nid);
    $target_arr = $target_locale_arr;
    foreach ($rows as $row) {
      if (array_search($row->target, $target_arr)) {
        unset($target_arr[$row->target]);
      }
    }

    if (empty($target_arr)) {
      transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_WARNING, 'Target languages already sent. Skipping nid - [' . $nid . ']');
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
      transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_WARNING, 'No target languages. Skipping nid - [' . $nid . ']');
      continue;
    }

    $drupal_target_arr = array();
    foreach ($target_arr as $target_locale) {
      array_push($drupal_target_arr, transperfect_get_drupal_locale_code($target_locale));
    }

    $tnid = NULL;
    $tvid = NULL;

    $name = '.xml';
    $xml = get_entity_xml($node, $drupal_target_arr, $tnid, $tvid, $name, FALSE, $drupal_locale_code);
    if (!$xml) {
      transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_WARNING, 'Cannot create XML. Skipping nid - [' . $nid . ']');
      continue;
    }

    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_DEBUG, $xml);

    $globallink = new GlobalLink();
    $globallink->nid = $node->nid;
    $globallink->vid = $node->vid;
    $globallink->title = $node->title;
    $globallink->type = $node->type;
    $globallink->metadata = 'Entity';
    $globallink->sourceLocale = $source_locale;
    $globallink->targetLocale = $target_arr;
    $globallink->sourceXML = $xml;
    $globallink->sourceFileName = $name;
    $globallink->submissionName = $submission_name;
    $globallink->dueDate = $due_date;
    $globallink->submissionInstructions = $submission_details['instructions'] . '\nSubmitter: ' . $submitter;
    $globallink_arr[] = $globallink;
  }

  if (!empty($globallink_arr)) {
    send_documents_for_translation_to_pd($globallink_arr, $pd4, $project_code, $submitter);
  }
  return $globallink_arr;
}

function update_entity_ticket_id($arr, $project_code) {
  foreach ($arr as $globallink) {
    $nid = $globallink->nid;
    $node = node_load($nid);
    $target_locale_arr = $globallink->targetLocale;
    $source_locale_code = transperfect_get_drupal_locale_code($globallink->sourceLocale);
    foreach ($target_locale_arr as $target_locale) {
      $row = get_transperfect_entity_row_by_nid_and_locale($nid, $globallink->sourceLocale, $target_locale);
      $title = $node->title;
      if ($row) {
        db_update('transperfect_core_entity')
                ->fields(array('vid' => $globallink->vid, 'title' => $title, 'document_ticket' => $globallink->documentTicket,
                    'submission' => $globallink->submissionName, 'submission_ticket' => $globallink->submissionTicket,
                    'status' => TPT_STATUS_SENT_FOR_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'last_modified' => $node->changed, 'changed' => 0, 'project_code' => $project_code))
                ->condition('rid', $row->rid, '=')
                ->execute();
      }
      else {
        db_insert('transperfect_core_entity')
                ->fields(array('nid' => $globallink->nid, 'vid' => $globallink->vid, 'type' => $globallink->type, 'title' => $title,
                    'source' => $globallink->sourceLocale, 'target' => $target_locale, 'document_ticket' => $globallink->documentTicket,
                    'submission' => $globallink->submissionName, 'submission_ticket' => $globallink->submissionTicket,
                    'status' => TPT_STATUS_SENT_FOR_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'last_modified' => $node->changed, 'changed' => 0, 'project_code' => $project_code))
                ->execute();
      }
    }
  }
}

function get_transperfect_entity_row_by_nid_and_locale($nid, $source, $target) {
  $result = db_select('transperfect_core_entity', 'tc')
          ->fields('tc')
          ->condition('nid', $nid, '=')
          ->condition('source', $source, '=')
          ->condition('target', $target, '=')
          ->execute();
  foreach ($result as $row) {
    return $row;
  }

  return FALSE;
}

function get_sent_tpt_entity_rows_by_nid($nid) {
  $result = db_select('transperfect_core_entity', 'tc')
          ->fields('tc')
          ->condition('nid', $nid, '=')
          ->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN')
          ->execute();
  $rows = array();
  foreach ($result as $row) {
    $rows[] = $row;
  }
  return $rows;
}

function update_entity_change_flag($nids, $source, $tgts) {
  transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_INFO, 'CHANGED Status Cleared for Node Ids - [' . implode(',', $nids) . ']');

  $insert_arr = array();
  $tgt_arr = array_keys($tgts);
  foreach ($tgt_arr as $tgt) {
    foreach ($nids as $nid) {
      $insert_arr[$tgt][$nid] = TRUE;
    }
  }
  foreach ($tgt_arr as $tgt) {
    foreach ($nids as $nid) {
      $result = db_select('transperfect_core_entity', 'tc')
              ->fields('tc')
              ->condition('nid', $nid, '=')
              ->condition('target', $tgt, '=')
              ->execute();
      foreach ($result as $item) {
        db_update('transperfect_core_entity')
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
          db_insert('transperfect_core_entity')
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

function cancel_select_entity_records($rowids, $pd4) {
  $globallink_arr = array();
  foreach ($rowids as $rid) {
    $row = get_transperfect_entity_row($rid);
    $globallink = new GlobalLink();
    $globallink->tptRowId = $row->rid;
    $globallink->targetLocale = $row->target;
    $globallink->documentTicket = $row->document_ticket;
    $globallink->submissionTicket = $row->submission_ticket;
    $globallink_arr[$rid] = $globallink;
  }

  cancel_select_documents($pd4, $globallink_arr);
  update_transperfect_entity_row_document($globallink_arr);
}

function cancel_entity_submission($selected_submission) {
  $pd4 = transperfect_get_project_director_details();
  $globallink = new GlobalLink();
  $submission_name = get_entity_submission_name($selected_submission);
  $globallink->submissionName = $submission_name;
  $globallink->submissionTicket = $selected_submission;

  cancel_pd_submission($pd4, $globallink);
  update_transperfect_entity_row_submission($globallink);
}

function get_entity_submission_name($submission_ticket) {
  $query = db_select('transperfect_core_entity', 'tc');
  $query->fields('tc');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $results = $query->execute();
  foreach ($results as $row) {
    if ($row->submission != '') {
      return $row->submission;
    }
  }
}

function update_transperfect_entity_row_document(&$globallink_arr) {
  foreach ($globallink_arr as $globallink) {
    if ($globallink->cancelled) {
      db_update('transperfect_core_entity')
              ->fields(array('status' => TPT_STATUS_PENDING_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'changed' => 1))
              ->condition('rid', $globallink->tptRowId, '=')
              ->execute();
    }
  }
}

function update_transperfect_entity_row_submission(&$globallink) {
  db_update('transperfect_core_entity')
          ->fields(array('status' => TPT_STATUS_PENDING_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'changed' => 1))
          ->condition('submission_ticket', $globallink->submissionTicket, '=')
          ->condition('submission', $globallink->submissionName, '=')
          ->execute();
}

function get_distinct_entity_active_submission_names() {
  $query = db_select('transperfect_core_entity', 'tc');
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

function get_tpt_entity_row_id_from_submission($submission_ticket, $document_ticket, $target_locale) {
  $query = db_select('transperfect_core_entity', 'tc');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $query->condition('document_ticket', $document_ticket, '=');
  $query->condition('target', $target_locale, '=');
  $query->fields('tc');
  $results = $query->execute();
  foreach ($results as $row) {
    // Return the first one
    return $row->rid;
  }
}

function update_deleted_entity_records($pd4, $globallink) {
  try {
    $globallink->status = TPT_STATUS_SOURCE_DELETED;
    send_download_confirmation($globallink->targetTicket, $pd4);
    update_tpt_entity_status($globallink);
  }
  catch (SoapFault $se) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, 'SOAP Exception in Sending Download Confirmation - update_deleted_records - Code[' . $se->faultcode . '], Message[' . $se->faultstring . ']');
    form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
  }
  catch (Exception $e) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, 'Exception in update_deleted_records - File[' . $e->getFile() . '], Line[' . $e->getLine() . '], Code[' . $e->getCode() . '], Message[' . $e->getMessage() . ']');
    form_set_error('', check_plain('Error: ' . $e->getMessage()));
  }

  return TRUE;
}

function get_translated_entity($pd4, &$globallink_arr) {
  try {
    $count = 0;
    foreach ($globallink_arr as $globallink) {
      if (!$globallink->sourceDeleted) {
        $globallink->targetXML = download_target_resource($pd4, $globallink->targetTicket);
        if (isset($globallink->targetXML)) {
          $count++;
          update_entity($globallink);
          if ($globallink->status != TPT_STATUS_ERROR) {
            send_download_confirmation($globallink->targetTicket, $pd4);
            update_tpt_entity_status($globallink);
          }
          else {
            $count--;
          }
        }
      }
    }
  }
  catch (SoapFault $se) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, 'SOAP Exception in Sending Download Confirmation - get_translated_content - Code[' . $se->faultcode . '], Message[' . $se->faultstring . ']');
    form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
  }
  catch (Exception $e) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, 'Exception in get_translated_content - File[' . $e->getFile() . '], Line[' . $e->getLine() . '], Code[' . $e->getCode() . '], Message[' . $e->getMessage() . ']');
    form_set_error('', check_plain('Error: ' . $e->getMessage()));
  }

  return $count;
}

function get_transperfect_entity_row($row_id) {
  $result = db_select('transperfect_core_entity', 'tc')
          ->fields('tc')
          ->condition('rid', $row_id, '=')
          ->execute();
  foreach ($result as $row) {
    return $row;
  }
}

function update_tpt_entity_status(&$globallink) {
  switch ($globallink->status) {
    case TPT_STATUS_SOURCE_DELETED:
    case TPT_STATUS_ERROR:
      $row = get_transperfect_entity_row_by_nid_and_locale($globallink->nid, $globallink->sourceLocale, $globallink->targetLocale);
      $node = node_load($row->nid);

      if ($node->vid != $row->vid) {
        db_update('transperfect_core_entity')
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
        db_update('transperfect_core_entity')
        ->fields(array(
          'status' => TPT_STATUS_PENDING_TRANSLATIONS,
          'timestamp' => REQUEST_TIME,
        ))
        ->condition('rid', $row->rid, '=')
        ->execute();
      }

      break;
    case TPT_STATUS_SOURCE_DELETED:
      $row = get_transperfect_entity_row_by_nid_and_locale($globallink->nid, $globallink->sourceLocale, $globallink->targetLocale);

      db_update('transperfect_core_entity')
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
      $row = get_transperfect_entity_row_by_nid_and_locale($globallink->nid, $globallink->sourceLocale, $globallink->targetLocale);
      db_update('transperfect_core_entity')
      ->fields(array(
        'status' => TPT_STATUS_ERROR,
        'timestamp' => REQUEST_TIME,
      ))
      ->condition('rid', $row->rid, '=')
      ->execute();

      break;
  }
}

function update_entity(&$globallink, $t_arr = NULL) {
  try {
    $target_lang = transperfect_get_drupal_locale_code($globallink->targetLocale);
    if ($t_arr != NULL) {
      $translated_arr = $t_arr;
    }
    else {
      $translated_arr = get_translated_entity_array($globallink->targetXML, $target_lang);
    }

    $target_nid = $translated_arr['nid'];
    $target_vid = $translated_arr['vid'];
    $globallink->nid = $target_nid;
    $globallink->vid = $target_vid;
    $node = node_load($target_nid, $target_vid);

    if (isset($translated_arr['title'])) {
      $node->title = $translated_arr['title'];
    }

    if ($node->title != $node->title_original) {
      $node->title = $node->title_original;
    }

    if (!$node || is_null($node) || !is_object($node)) {
      $globallink->status = TPT_STATUS_SOURCE_DELETED;
      return;
    }

    $nodepath = drupal_lookup_path('alias', 'node/' . $target_nid);
    if ($nodepath) {
      if (!empty($translated_arr['path'])) {
        $path = array('alias' => $translated_arr['path'], 'source' => 'node/' . $target_nid, 'language' => $target_lang);
      }
      else {
        $path = array('alias' => $nodepath, 'source' => 'node/' . $target_nid, 'language' => $target_lang);
      }

      if (module_exists('pathauto')) {
        $path['pathauto'] = FALSE;
      }

      path_save($path);
    }

    $node->revision = 1;
    if (module_exists('revisioning')) {
      $node->is_pending = FALSE;
      $node->revision_moderation = FALSE;
    }

    // Node presave hook skip
    $node->tpt_skip = TRUE;

    if (module_exists('metatag')) {
      if (isset($translated_arr['metatag'])) {
        $target_metatag_arr = $translated_arr['metatag'];
        if (!empty($node->metatags[$node->language])) {
          $metatags_names = array_keys($node->metatags[$node->language]);
          $n_metatag = &$node->metatags;
          foreach ($metatags_names as $name) {
            if (isset($target_metatag_arr[$name]) && isset($target_metatag_arr[$name][0])) {
              $gl_obj = $target_metatag_arr[$name]['0'];
              if (is_object($gl_obj)) {
                $translated_content = $gl_obj->translatedContent;
              }
              else {
                $translated_content = $gl_obj;
              }
              $n_metatag[$target_lang][$name] = array('value' => $translated_content);
            }
          }
        }
      }
    }

    $success = save_translated_entity_with_fields($node, $translated_arr, $target_lang);
    if ($success) {
      // todo Pathauto Module Support
      $globallink->status = TPT_STATUS_PUBLISHED;
    }
    else {
      $globallink->status = TPT_STATUS_ERROR;
    }
  }
  catch (Exception $e) {
    $globallink->status = TPT_STATUS_ERROR;
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, 'Exception in Receive Translations - Update Node - File[' . $e->getFile() . '], Line[' . $e->getLine() . '], Code[' . $e->getCode() . '], Message[' . $e->getMessage() . ']');
  }
}

function get_translated_entity_array($xml, $target_lang = LANGUAGE_NONE) {
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
          $arr[$field_image_obj->field_name][$target_lang][$field_image_obj->delta]->title = $field_image_obj->title;
        }
        if (isset($field_image_obj->alt)) {
          $arr[$field_image_obj->field_name][$target_lang][$field_image_obj->delta]->alt = $field_image_obj->alt;
        }
      }
      else {
        $arr[$field_image_obj->field_name][$target_lang][$field_image_obj->delta] = $field_image_obj;
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
        }
      }
      if (is_null($field_obj->entityId)) {
        $field_obj->entityId = '0';
      }
      if (is_null($field_obj->bundle)) {
        $field_obj->bundle = $field_obj->fieldName;
      }
      if (is_null($field_obj->delta)) {
        $field_obj->delta = '0';
      }
      if ($field_obj->entityType == 'node') {
        $arr[$field_obj->fieldName][$target_lang][$field_obj->delta] = $field_obj;
      }
      else {
        $arr['field_collection'][$field_obj->parentFCName][$field_obj->bundle][$field_obj->entityId][$field_obj->fieldName][$target_lang][$field_obj->delta] = $field_obj;
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

function get_active_entity_submission_by_nid($nid) {
  $query = db_select('transperfect_core_entity', 'tc');
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

function get_active_entity_submission_rows_by_nid($nids) {
  $query = db_select('transperfect_core_entity', 'tc');
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

function transperfect_entity_save(&$node) {
  try {
    node_save($node);
    return TRUE;
  }
  catch (Exception $e) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, 'Error in creating a translated node - File[' . $e->getFile() . '], Line[' . $e->getLine() . '], Code[' . $e->getCode() . '], Message[' . $e->getMessage() . ']');
    return FALSE;
  }
}

function save_translated_entity_with_fields(&$node, $translation_arr, $target_lang) {
  $config_fields = transperfect_get_config_fields($node->type);
  foreach ($config_fields as $field_name => $field_info) {
    if ($field_info->field_type == 'image' && $field_info->translatable == 1) {
      $source_lang = $node->language;
      $node->{$field_name}[$target_lang] = $node->{$field_name}[$source_lang];
    }
  }

  foreach ($translation_arr as $field => $values) {
    if (!is_array($values)) {
      continue;
    }

    foreach ($values as $target => $arr_value) {
      foreach ($arr_value as $obj) {
        if (!isset($obj->translatedContent)) {
          continue;
        }

        $format = '';
        if (!empty($obj->format)) {
          $format = $obj->format;
        }

        if (isset($node->{$field}[$target])) {
          $node->{$field}[$target][$obj->delta] = array(
              'value' => $obj->translatedContent, 'format' => $format);
        }
        else {
          $node->{$field}[$target] = array($obj->delta => array(
              'value' => $obj->translatedContent, 'format' => $format));
        }
      }
    }
  }

  $is_hook_enabled = variable_get('transperfect_implementation_type', 0);
  if ($is_hook_enabled == 1) {
    transperfect_update_node($node, $node);
  }

  // Set new translation data
  $node->translations->data[$target_lang] = array(
    'entity_type' => 'node',
    'entity_id' => $node->nid,
    'language' => $target_lang, // Target language
    'source' => $node->language, // Source language
    'uid' => $node->uid,
    'status' => variable_get('transperfect_publish_node', 0), // publish translation
    'translate' => 0, // Translation out of date
    'created' => $node->created,
    'changed' => $node->changed, // Date of the first node save
  );

  // Save new translation data
  $success = transperfect_entity_save($node);
  return $success;
}

function save_entity_field_collections($node, $t_arr, $target_lang) {
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
              save_entity_field_collections_recursively('node', $node, $fc_entity_id, $t_arr['field_collection'][$field], $target_lang);
            }
          }
        }
      }
    }
  }
}

function save_entity_field_collections_recursively($entity_type, $host_entity, $fc_entity_id, $translated_fc_arr, $target_lang) {
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
    if (!empty($fc_field_def) && isset($fc_field_def['type'])) {
      if ($fc_field_def['type'] != 'list_boolean'
              && $fc_field_def['type'] != 'file'
              && $fc_field_def['type'] != 'taxonomy_term_reference'
              && $fc_field_def['type'] != 'field_collection') {

        if (isset($field_collection_item_array[$key]) && isset($field_collection_item_array[$key][LANGUAGE_NONE])) {
          $field_data_arr = $field_collection_item_array[$key][LANGUAGE_NONE];
          foreach ($field_data_arr as $delta => $field_data) {
            if (isset($translated_fc_arr[$field_collection_name][$fc_entity_id][$key][$target_lang][$delta])) {
              $gl_obj = $translated_fc_arr[$field_collection_name][$fc_entity_id][$key][$target_lang][$delta];
              if (is_object($gl_obj)) {
                $translated_content = $gl_obj->translatedContent;
              }
              else {
                $translated_content = $gl_obj;
              }

              $new_field_collection_arr[$key][$target_lang][$delta] = array('value' => $translated_content);
              if (isset($field_data['format'])) {
                $new_field_collection_arr[$key][$target_lang][$delta]['format'] = $field_data['format'];
              }
            }
          }
        }
      }
      elseif ($fc_field_def['type'] != 'field_collection') {
        if (isset($field_collection_item_array[$key]) && isset($field_collection_item_array[$key][$target_lang])) {
          $field_data_arr = $field_collection_item_array[$key][$target_lang];
          $new_field_collection_arr[$key][$target_lang] = $field_data_arr;
        }
      }
    }
  }

  if ($entity_type != 'node') {
    $host_entity->tpt_skip = TRUE;
  }

  $new_entity = entity_create('field_collection_item', $new_field_collection_arr); // Create new field collection item.
  $new_entity->setHostEntity($entity_type, $host_entity); // Attach it to the node.
  $new_entity->save(TRUE); // Save field-collection item

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
              save_entity_field_collections_recursively('field_collection_item', $new_entity, $fc_entity_id, $translated_fc_arr, $target_lang);
            }
          }
        }
      }
    }
  }
}

function get_entity_xml($node, $target_arr, $tnid = NULL, $tvid = NULL, &$name = '', $for_display = FALSE, $source_lang = '') {
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
    $xml = generate_entity_xml_document($node, $target_arr, $tnid, $tvid, $for_display, $source_lang);
    return $xml;
  }

  return TPT_STATUS_SOURCE_DELETED;
}

function generate_entity_xml_document($node, $target_arr, $tnid = NULL, $tvid = NULL, $for_display = FALSE, $source_lang = '') {
  $is_hook_enabled = variable_get('transperfect_implementation_type', 0);

  try {
    $dom = new DOMDocument('1.0', 'UTF-8');
    $dom->formatOutput = TRUE;

    $root = $dom->createElement('content');
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
    $path = path_load(array('source' => 'node/' . $node->nid, 'language' => $source_lang));
    if (isset($path['alias'])) {
      $url->value = url($path['alias'], array('absolute' => TRUE));
    }
    else {
      $url->value = url('node/' . $node->nid, array('absolute' => TRUE));
    }

    if (!empty($path)) {
      transperfect_insert_child_element($dom, $root, 'path', $path['alias']);
    }

    $root->appendChild($url);
    $dom->appendChild($root);
    $field_arr = field_info_instances('node', $node->type);
    $keys = array_keys($field_arr);
    foreach ($keys as $field) {
      $field_def = field_read_field($field);
      $items = field_get_items('node', $node, $field, $source_lang);
      if ($items) {
        $parent_fc = '';
        if ($field_def['type'] == 'field_collection') {
          $parent_fc = $field;
          if (isset($items[0]['revision_id'])) {
            entity_load('field_collection_item', array($items[0]['value']), array('revision_id' => $items[0]['revision_id']));
          }
        }
        traverse_entity_fields_and_field_collections('node', $node->type, $parent_fc, $node->type, $node->nid, $items, $field, $field_def, $dom, $root, $node, $target_arr, $is_hook_enabled);
      }
    }

    if (module_exists('metatag')) {
      if (isset($node->metatags)) {
        if ($for_display) {
          if (!empty($node->metatags[$node->language])) {
            $metatags = $node->metatags[$node->language];
            foreach ($metatags as $name => $value) {
              if (isset($value['value'])) {
                transperfect_insert_child_element($dom, $root, 'metatag', $value['value'], array('entity_type' => 'node', 'name' => $name, 'label' => 'Metatag - ' . ucwords($name)));
              }
            }
          }
        }
        elseif (transperfect_is_field_configured_for_translation('node', $node->type, 'metatags', $node->type)) {
          if (!empty($node->metatags[$node->language])) {
            $metatags = $node->metatags[$node->language];
            foreach ($metatags as $name => $value) {
              if (isset($value['value'])) {
                transperfect_insert_child_element($dom, $root, 'metatag', $value['value'], array('entity_type' => 'node', 'name' => $name, 'label' => 'Metatag - ' . ucwords($name)));
              }
            }
          }
        }
      }
    }
  }
  catch (Exception $e) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, 'Exception in creating XML - File[' . $e->getFile() . '], Line[' . $e->getLine() . '], Code[' . $e->getCode() . '], Message[' . $e->getMessage() . ']');
    throw $e;
  }

  $root_element = $dom->getElementsByTagName('content')->item(0);
  if (!$root_element->hasChildNodes()) {
    return FALSE;
  }

  return $dom->saveXML();
}

function traverse_entity_fields_and_field_collections($entity_type, $content_type, $parent_fc, $bundle, $entity_id, $items, $field, $field_def, $dom, $root, $source_node, $target_arr, $is_hook_enabled = 0) {
  if ($items) {
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
                transperfect_insert_child_element($dom, $root, 'field', $f_value, array('entity_type' => $entity_type, 'content_type' => $content_type, 'parent_fc' => $parent_fc, 'bundle' => $bundle, 'entity_id' => $entity_id, 'field_name' => $field, 'label' => $f_label['label'], 'delta' => $delta, 'format' => $f_format));
              }
              elseif ($field_def['type'] == 'link_field' && isset($item['title']) && is_string($item['title'])) {
                $f_label = field_info_instance($entity_type, $field, $bundle);
                $f_value = $item['title'];
                $f_format = (isset($item['format']) && !is_null($item['format'])) ? $item['format'] : '';
                transperfect_insert_child_element($dom, $root, 'field', $f_value, array('entity_type' => $entity_type, 'content_type' => $content_type, 'parent_fc' => $parent_fc, 'bundle' => $bundle, 'entity_id' => $entity_id, 'field_name' => $field, 'label' => $f_label['label'], 'delta' => $delta, 'format' => $f_format));
              }
              elseif ($field_def['type'] == 'image') {
                $f_label = field_info_instance($entity_type, $field, $bundle);
                if (isset($item['alt'])) {
                  $f_value = $item['alt'];
                  transperfect_insert_child_element($dom, $root, 'field_image', $f_value, array('field_name' => $field, 'type' => 'alt', 'delta' => $delta));
                }
                if (isset($item['title'])) {
                  $f_value = $item['title'];
                  transperfect_insert_child_element($dom, $root, 'field_image', $f_value, array('field_name' => $field, 'type' => 'title', 'delta' => $delta));
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
                transperfect_insert_child_element($dom, $root, 'field', $f_value, array('entity_type' => $entity_type, 'content_type' => $content_type, 'parent_fc' => $parent_fc, 'bundle' => $bundle, 'entity_id' => $entity_id, 'field_name' => $field, 'label' => $f_label['label'], 'delta' => $delta, 'format' => $f_format));
              }
              elseif ($field_def['type'] == 'link_field' && isset($item['title']) && is_string($item['title'])) {
                $f_label = field_info_instance($entity_type, $field, $bundle);
                $f_value = $item['title'];
                $f_format = (isset($item['format']) && !is_null($item['format'])) ? $item['format'] : '';
                transperfect_insert_child_element($dom, $root, 'field', $f_value, array('entity_type' => $entity_type, 'content_type' => $content_type, 'parent_fc' => $parent_fc, 'bundle' => $bundle, 'entity_id' => $entity_id, 'field_name' => $field, 'label' => $f_label['label'], 'delta' => $delta, 'format' => $f_format));
              }
            }
          }
        }
      }
      elseif (module_exists('field_collection')) {
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
                    traverse_entity_fields_and_field_collections('field_collection_item', $content_type, $parent_fc, $field_collection_name, $fc_entity_id, $fc_item, $key, $fc_field_def, $dom, $root, $source_node, $target_arr, $is_hook_enabled);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

function get_entity_title(&$globallink) {
  $result = db_select('transperfect_core_entity', 'tc')
          ->fields('tc')
          ->condition('document_ticket', $globallink->documentTicket, '=')
          ->condition('submission_ticket', $globallink->submissionTicket, '=')
          ->execute();
  foreach ($result as $item) {
    $nid = $item->nid;
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

    return l(transperfect_format_display_string($globallink->title), 'node/' . $item->nid);
  }
  return FALSE;
}

function update_submission_entity_status($submission_ticket, $status = TPT_STATUS_CANCELLED) {
  db_update('transperfect_core_entity')
            ->fields(array(
                'status' => $status,
                'timestamp' => REQUEST_TIME
                )
            )
            ->condition('submission_ticket', $submission_ticket, '=')
            ->execute();
}

function get_entity_submission_status() {
  $query = db_select('transperfect_core_entity', 'tc');
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
          update_submission_entity_status($row->submission_ticket);
        }
      }
      catch (SoapFault $se) {
        update_submission_entity_status($row->submission_ticket);
      }
      catch (Exception $ex) {
        update_submission_entity_status($row->submission_ticket);
      }
    }
  }
}

function check_tpt_entity_status($rids_arr) {
  $status = TRUE;
  $query = db_select('transperfect_core_entity', 'tc')
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

function clear_cancelled_entity_documents() {
  $count = 0;
  $query = db_select('transperfect_core_entity', 'tc')
          ->fields('tc', array('submission_ticket'))
          ->distinct()
          ->condition('status', TPT_STATUS_CANCELLED, '=');
  $results = $query->execute();
  foreach ($results as $item) {
    update_submission_entity_status($item->submission_ticket, TPT_STATUS_PENDING_TRANSLATIONS);
    $count++;
  }

  return $count;
}
