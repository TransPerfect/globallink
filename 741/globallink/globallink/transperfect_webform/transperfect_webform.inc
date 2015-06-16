<?php

require_once DRUPAL_ROOT . ('/includes/locale.inc');
require_once TPT_ROOT . '/' . ('transperfect_settings.inc');
require_once TPT_ROOT . '/' . ('transperfect_common.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_common.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_receive_translations.inc');

function send_webform_for_translations($lids, $pd4, $submission_name, $due_date, $project_code, $source_locale, $target_locale_arr, $submission_details) {
  $globalLink_arr = array();
  $submitter = $submission_details['submitter'];
  foreach ($lids as $lid) {
    $rows = get_sent_tpt_webform_rows_by_lid($lid);
    $target_arr = $target_locale_arr;
    foreach ($rows as $row) {
      if (array_search($row->target, $target_locale_arr)) {
        unset($target_arr[$row->target]);
      }
    }
    if (empty($target_arr)) {
      continue;
    }
    $xml = get_webform_xml($lid);
    $name = "Webform_" . $lid . ".xml";
    $globalLink = new GlobalLink();
    $globalLink->type = 'webform';
    $globalLink->metadata = 'Webform';
    $globalLink->sourceLocale = $source_locale;
    $globalLink->targetLocale = $target_arr;
    $globalLink->sourceXML = $xml;
    $globalLink->sourceFileName = $name;
    $globalLink->submissionName = $submission_name;
    $globalLink->dueDate = $due_date;
    $globalLink->otherObjectId = $lid;
    $globalLink->submissionInstructions = $submission_details['instructions'] . "\nSubmitter: " . $submitter;
    $globalLink_arr[] = $globalLink;
  }

  if (!empty($globalLink_arr)) {
    sendDocumentsForTranslationToPD($globalLink_arr, $pd4, $project_code, $submitter);
  }
  return $globalLink_arr;
}

function get_webform_xml($lid) {
  $property_arr = array();
  $string_result = db_select('i18n_string', 's')
          ->fields('s')
          ->condition('lid', $lid, '=')
          ->execute();
  foreach ($string_result as $row) {
    $property_arr['property'] = $row->property;
  }

  $dom = new DOMDocument('1.0', 'UTF-8');
  $dom->formatOutput = TRUE;

  $root = $dom->createElement("content");
  $dom->appendChild($root);

  $id = $dom->createAttribute('lid');
  $id->value = $lid;
  $root->appendChild($id);

  $result = db_select('locales_source', 'ls')
          ->fields('ls')
          ->condition('lid', $lid, '=')
          ->execute();

  foreach ($result as $row) {
    insert_child_element($dom, $root, "webform", $row->source, array('name' => $property_arr['property'], 'lid' => $lid,
        'location' => $row->location,
        'format' => isset($lid['format']) ? $lid['format'] : ''));
  }

  $xml = $dom->saveXML();
  return $xml;
}

function update_webform_ticket_id($arr, $project_code) {
  foreach ($arr as $globalLink) {
    $target_locale_arr = $globalLink->targetLocale;
    $type = $globalLink->type;
    if ($type == 'webform') {
      $lid = $globalLink->otherObjectId;
      $webform = load_source_data($lid);
      foreach ($target_locale_arr as $target_locale) {
        $row = get_transperfect_webform_row($lid, $type, $globalLink->sourceLocale, $target_locale);
        if ($row) {
          db_update('transperfect_core_webform')
                  ->fields(array('title' => $webform[0]->source, 'document_ticket' => $globalLink->documentTicket,
                      'submission' => $globalLink->submissionName, 'submission_ticket' => $globalLink->submissionTicket,
                      'status' => TPT_STATUS_SENT_FOR_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'last_modified' => REQUEST_TIME, 'project_code' => $project_code))
                  ->condition('rid', $row->rid, '=')
                  ->execute();
        }
        else {
          db_insert('transperfect_core_webform')
                  ->fields(array('object_id' => $lid, 'object_type' => $globalLink->type, 'title' => $webform[0]->source,
                      'source' => $globalLink->sourceLocale, 'target' => $target_locale, 'document_ticket' => $globalLink->documentTicket,
                      'submission' => $globalLink->submissionName, 'submission_ticket' => $globalLink->submissionTicket,
                      'status' => TPT_STATUS_SENT_FOR_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'last_modified' => REQUEST_TIME, 'project_code' => $project_code))
                  ->execute();
        }
      }
    }
  }
}

function update_deleted_webform_records($pd4, $globalLink) {
  try {
    $globalLink->status = TPT_STATUS_SOURCE_DELETED;
    sendDownloadConfirmation($globalLink->targetTicket, $pd4);
    update_tpt_webform_status($globalLink, TPT_STATUS_SOURCE_DELETED);
  }
  catch (SoapFault $se) {
    gl_log(TPT_OBJECT_TYPE_WEBFORM, TPT_LOGGING_SEVERITY_ERROR, "SOAP Exception in Sending Download Confirmation - update_deleted_webform_records - Code[$se->faultcode], Message[$se->faultstring]");
    form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
    return FALSE;
  }
  catch (Exception $e) {
    gl_log(TPT_OBJECT_TYPE_WEBFORM, TPT_LOGGING_SEVERITY_ERROR, "Exception in update_deleted_block_records - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
    form_set_error('', check_plain('Error: ' . $e->getMessage()));
    return FALSE;
  }

  return TRUE;
}

function get_translated_webforms($pd4, &$globalLink_arr) {
  $count = 0;
  foreach ($globalLink_arr as $globalLink) {
    $targetXML = downloadTargetResource($pd4, $globalLink->targetTicket);
    if (!$globalLink->sourceDeleted) {
      if (isset($targetXML)) {
        $count++;
        $language = get_drupal_locale_code($globalLink->targetLocale);
        $translated_arr = get_translated_webform_items($targetXML);
        try {
          $lid = $translated_arr['lid'];
          foreach ($translated_arr as $attribute => $translations) {
            if ($attribute == 'lid') {
              continue;
            }
            $webform = '';
            if ($attribute == '#title' || $attribute == '#description') {
              $webform = load_source_data($translations['lid']);
              if ($webform == '') {
                throw new Exception("Source string not found for webform id $lid and field name $attribute");
              }
            }
            $report = &drupal_static(__FUNCTION__, array('additions' => 0, 'updates' => 0, 'deletes' => 0, 'skips' => 0));
            _locale_import_one_string_db($report, $language, $webform[0]->context, $webform[0]->source, $translations['translation'], 'webform', $translations['location'], LOCALE_IMPORT_OVERWRITE);
          }
          if ($globalLink->status != TPT_STATUS_ERROR) {
            if ($globalLink->status != TPT_STATUS_DELETED) {
              sendDownloadConfirmation($globalLink->targetTicket, $pd4);
            }
            update_tpt_webform_status($globalLink);
          }
          else {
            $count--;
          }
        }
        catch (SoapFault $se) {
          $count--;
          update_tpt_webform_status($globalLink, TPT_STATUS_ERROR);
          form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
        }
        catch (Exception $ex) {
          $count--;
          update_tpt_webform_status($globalLink, TPT_STATUS_ERROR);
          form_set_error('', check_plain('Error: ' . $ex->getMessage()));
        }
      }
    }
  }
  return $count;
}

function get_webform_translate_filter_query() {
  $filter = array();
  if (isset($_SESSION['transperfect_webform_filter'])) {
    foreach ($_SESSION['transperfect_webform_filter'] as $key => $value) {
      $filter[$key] = $value;
    }
  }
  return $filter;
}

function get_webform_submission_status($lid, $locale_code) {
  $result = db_select('transperfect_core_webform', 'tcw')
          ->fields('tcw', array('status'))
          ->condition('object_id', $lid, '=')
          ->condition('target', $locale_code, '=')
          ->execute();

  while ($record = $result->fetchAssoc()) {
    return $record;
  }
}

function webform_locale_languages($lid) {
  $mapped_locales = get_mapped_tpt_locales_with_drupal_desc(TRUE);
  $translation_status = '';
  foreach ($mapped_locales as $code => $locale) {
    $result = get_webform_submission_status($lid, $code);
    if ($result) {
      $translation_status .= $locale . " - " . $result['status'] . "<br />";
    }
  }

  return $translation_status;
}

function get_sent_tpt_webform_rows_by_lid($lid) {
  $result = db_select('transperfect_core_webform', 'tcw')
          ->fields('tcw')
          ->condition('object_id', $lid, '=')
          ->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN')
          ->execute();
  $rows = array();
  foreach ($result as $row) {
    $rows[] = $row;
  }

  return $rows;
}

function update_tpt_webform_status(&$globalLink, $status = TPT_STATUS_PENDING_TRANSLATIONS) {
  db_update('transperfect_core_webform')
          ->fields(array('status' => $status, 'timestamp' => REQUEST_TIME))
          ->condition('submission', $globalLink->submissionName, '=')
          ->condition('document_ticket', $globalLink->documentTicket, '=')
          ->condition('source', $globalLink->sourceLocale, '=')
          ->condition('target', $globalLink->targetLocale, '=')
          ->execute();
}

function get_transperfect_webform_row($object_id, $object_type, $source, $target) {
  $result = db_select('transperfect_core_webform', 'tcw')
          ->fields('tcw')
          ->condition('object_id', $object_id, '=')
          ->condition('object_type', $object_type, '=')
          ->condition('source', $source, '=')
          ->condition('target', $target, '=')
          ->execute();
  foreach ($result as $row) {
    return $row;
  }

  return FALSE;
}

function get_webform_active_submission_names() {
  $query = db_select('transperfect_core_webform', 'tcw');
  $query->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $query->distinct();
  $query->fields('tcw');
  $results = $query->execute();
  $arr = array('' => '-- Select a Submission --');
  foreach ($results as $row) {
    $arr[$row->submission_ticket] = $row->submission;
  }
  return $arr;
}

function get_webform_submission_name($submission_ticket) {
  $query = db_select('transperfect_core_webform', 'tcw');
  $query->fields('tcw');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $results = $query->execute();
  foreach ($results as $row) {
    if ($row->submission != '') {
      return $row->submission;
    }
  }
}

function cancel_webform_submission($selected_submission) {
  $pd4 = get_project_director_details();
  $globalLink = new GlobalLink();
  $submission_name = get_webform_submission_name($selected_submission);
  $globalLink->submissionName = $submission_name;
  $globalLink->submissionTicket = $selected_submission;

  cancel_ProjectDirector_submission($pd4, $globalLink);
  update_transperfect_webform_submission($globalLink);
}

function cancel_webform_records($rowids, $pd4) {
  $globalLink_arr = array();
  foreach ($rowids as $rid) {
    $row = get_transperfect_webform_other_row($rid);
    $globalLink = new GlobalLink();
    $globalLink->tptRowId = $row->rid;
    $globalLink->targetLocale = $row->target;
    $globalLink->documentTicket = $row->document_ticket;
    $globalLink->submissionTicket = $row->submission_ticket;
    $globalLink_arr[$rid] = $globalLink;
  }

  cancel_select_documents($pd4, $globalLink_arr);
  update_webform_row_document($globalLink_arr);
}

function get_transperfect_webform_other_row($row_id) {
  $result = db_select('transperfect_core_webform', 'tcw')
          ->fields('tcw')
          ->condition('rid', $row_id, '=')
          ->execute();
  foreach ($result as $row) {
    return $row;
  }
}

function update_transperfect_webform_submission(&$globalLink) {
  db_update('transperfect_core_webform')
          ->fields(array('status' => TPT_STATUS_PENDING_TRANSLATIONS, 'timestamp' => REQUEST_TIME))
          ->condition('submission_ticket', $globalLink->submissionTicket, '=')
          ->condition('submission', $globalLink->submissionName, '=')
          ->execute();
}

function update_webform_row_document(&$globalLink_arr) {
  foreach ($globalLink_arr as $globalLink) {
    if ($globalLink->cancelled) {
      db_update('transperfect_core_webform')
              ->fields(array('status' => TPT_STATUS_PENDING_TRANSLATIONS, 'timestamp' => REQUEST_TIME))
              ->condition('rid', $globalLink->tptRowId, '=')
              ->execute();
    }
  }
}

function get_distinct_active_webform_submission_names() {
  $query = db_select('transperfect_core_webform', 'tcw');
  $query->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $query->distinct();
  $query->fields('tcw');
  $results = $query->execute();
  $arr = array('[any]' => t(' Any '));
  foreach ($results as $row) {
    $arr[$row->submission] = $row->submission;
  }
  return $arr;
}

function get_translated_webform_items($xml) {
  if (is_null($xml) || !is_string($xml) || $xml == '') {
    return array();
  }

  $dom = new DomDocument;
  $dom->preserveWhiteSpace = FALSE;
  $dom->loadXML($xml);

  $contents = $dom->getElementsByTagName('content');

  $lid = '';
  foreach ($contents as $content) {
    if (!is_null($content->attributes)) {
      foreach ($content->attributes as $attrName => $attrNode) {
        if ('lid' == $attrName) {
          $lid = $attrNode->value;
        }
      }
    }
  }

  if ($lid == '') {
    return array();
  }

  $webform_arr = array();
  $webform_arr['lid'] = $lid;

  $webforms = $dom->getElementsByTagName('webform');
  foreach ($webforms as $webform) {
    if (!is_null($webform->attributes)) {
      $w_arr = array();
      foreach ($webform->attributes as $attrName => $attrNode) {
        $w_arr[$attrName] = $attrNode->value;
      }
      $w_arr['translation'] = $webform->nodeValue;
      $webform_arr[$w_arr['name']] = $w_arr;
    }
  }

  return $webform_arr;
}

function get_webform_translation_status($lid, $tgt_locale, &$title) {
  $query = db_select('transperfect_core_webform', 'tcw');
  $query->condition('object_id', $lid, '=');
  $query->condition('target', $tgt_locale, '=');
  $query->distinct();
  $query->fields('tcw');
  $results = $query->execute();
  foreach ($results as $row) {
    $title = $row->title;
    $webform = load_source_data($lid);
    if (!$webform || is_null($webform))
      return TPT_STATUS_SOURCE_DELETED;
    elseif ($row->status == TPT_STATUS_SENT_FOR_TRANSLATIONS)
      return TPT_STATUS_COMPLETED;
    elseif ($row->status == TPT_STATUS_ERROR)
      return TPT_STATUS_ERROR;
  }

  return TPT_STATUS_SENT_FOR_TRANSLATIONS;
}

function get_sent_tpt_webform_rows_by_lid_target($lid, $target) {
  $result = db_select('transperfect_core_webform', 'tcw')
          ->fields('tcw')
          ->condition('object_id', $lid, '=')
          ->condition('target', $target, '=')
          ->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN')
          ->execute();
  $rows = array();
  foreach ($result as $row) {
    $rows[] = $row;
  }
  return $rows;
}

function get_tpt_webform_row_id_from_submission($submission_ticket, $document_ticket, $target_locale) {
  $query = db_select('transperfect_core_webform', 'tc');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $query->condition('document_ticket', $document_ticket, '=');
  $query->condition('target', $target_locale, '=');
  $query->fields('tc');
  $results = $query->execute();
  foreach ($results as $row) {
    return $row->rid;
  }
}

function get_webform_active_submission_rows_by_lid($lids) {
  $query = db_select('transperfect_core_webform', 'tci');
  $query->condition('status', TPT_STATUS_SENT_FOR_TRANSLATIONS, '=');
  $query->condition('object_id', $lids, 'IN');
  $query->fields('tci');
  $results = $query->execute();
  $arr = array();
  foreach ($results as $row) {
    if (array_key_exists($row->object_id, $arr)) {
      array_push($arr[$row->object_id], $row);
    }
    else {
      $arr[$row->object_id] = array($row);
    }
  }
  $final_arr = array();
  foreach ($arr as $lid => $lid_arr) {
    $sub_arr = array();
    foreach ($lid_arr as $r) {
      if (array_key_exists($r->submission, $sub_arr)) {
        array_push($sub_arr[$r->submission], $r->target);
      }
      else {
        $sub_arr[$r->submission] = array($r->target);
      }
    }
    if (count($sub_arr) > 0) {
      $final_arr[$lid] = $sub_arr;
    }
  }
  if (count($final_arr) > 0) {
    return $final_arr;
  }
  return FALSE;
}

function update_webform_submission_status($submission_ticket, $status = TPT_STATUS_CANCELLED) {
  db_update('transperfect_core_webform')
          ->fields(array(
              'status' => $status,
              'timestamp' => REQUEST_TIME
                  )
          )
          ->condition('submission_ticket', $submission_ticket, '=')
          ->execute();
}

function get_webform_pd_submission_status() {
  $query = db_select('transperfect_core_webform', 'tc');
  $query->fields('tc', array('submission_ticket'));
  $query->distinct();
  $query->condition('status', TPT_STATUS_SENT_FOR_TRANSLATIONS, '=');
  $results = $query->execute();
  foreach ($results as $row) {
    if ($row->submission_ticket) {
      try {
        $pd4 = get_project_director_details();
        $doc_status = get_status($pd4, $row->submission_ticket);
        if (!$doc_status || $doc_status == PD_STATUS_CANCELLED) {
          update_webform_submission_status($row->submission_ticket);
        }
      }
      catch (SoapFault $se) {
        update_webform_submission_status($row->submission_ticket);
      }
      catch (Exception $ex) {
        update_webform_submission_status($row->submission_ticket);
      }
    }
  }
}

function check_tpt_webform_status($rids_arr) {
  $status = TRUE;
  $query = db_select('transperfect_core_webform', 'tc')
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

function clear_cancelled_webform_documents() {
  $count = 0;
  $query = db_select('transperfect_core_webform', 'tc')
          ->fields('tc', array('submission_ticket'))
          ->distinct()
          ->condition('status', TPT_STATUS_CANCELLED, '=');
  $results = $query->execute();
  foreach ($results as $item) {
    update_webform_submission_status($item->submission_ticket, TPT_STATUS_PENDING_TRANSLATIONS);
    $count++;
  }

  return $count;
}