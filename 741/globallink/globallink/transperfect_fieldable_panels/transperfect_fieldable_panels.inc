<?php

require_once DRUPAL_ROOT . ('/includes/locale.inc');
require_once TPT_ROOT . '/' . ('transperfect_settings.inc');
require_once TPT_ROOT . '/' . ('transperfect_common.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_common.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_receive_translations.inc');

function send_fieldable_panels_pane_for_translations($fpids, $pd4, $submission_name, $due_date, $project_code, $source_locale, $target_locale_arr, $submission_details) {
  $submitter = $submission_details['submitter'];
  $globalLink_arr = array();
  foreach ($fpids as $fpid) {
    $rows = get_sent_tpt_fieldable_panels_rows_by_fpid($fpid);
    $target_arr = $target_locale_arr;
    foreach ($rows as $row) {
      if (array_search($row->target, $target_locale_arr)) {
        unset($target_arr[$row->target]);
      }
    }

    if (empty($target_arr)) {
      continue;
    }

    $xml = get_fieldable_panels_pane_xml($fpid);
    $name = "Fieldable_Panels_Panes_" . $fpid . ".xml";
    $globalLink = new GlobalLink();
    $globalLink->type = 'fieldable_panels_panes';
    $globalLink->metadata = 'Fieldable_Panels_Panes';
    $globalLink->sourceLocale = $source_locale;
    $globalLink->targetLocale = $target_arr;
    $globalLink->sourceXML = $xml;
    $globalLink->sourceFileName = $name;
    $globalLink->submissionName = $submission_name;
    $globalLink->dueDate = $due_date;
    $globalLink->otherObjectId = $fpid;
    $globalLink->submissionInstructions = $submission_details['instructions'] . "\nSubmitter: " . $submitter;
    $globalLink_arr[] = $globalLink;
  }

  if (!empty($globalLink_arr)) {
    sendDocumentsForTranslationToPD($globalLink_arr, $pd4, $project_code, $submitter);
  }
  return $globalLink_arr;
}

function get_fieldable_panels_pane_xml($fpid) {
  $dom = new DOMDocument('1.0', 'UTF-8');
  $dom->formatOutput = TRUE;

  $root = $dom->createElement("content");
  $dom->appendChild($root);

  $id = $dom->createAttribute('fpid');
  $id->value = $fpid;
  $root->appendChild($id);

  $fieldable_panels_pane = fieldable_panels_panes_load($fpid);
  $language = $fieldable_panels_pane->language;

  $fields = get_fieldable_panels_pane_fields();
  foreach ($fields as $field) {
    if (!empty($fieldable_panels_pane->$field)) {
      $field_values = $fieldable_panels_pane->$field;
      if (is_array($field_values) && !empty($field_values[$language])) {
        foreach ($field_values[$language] as $delta => $values) {
          insert_child_element($dom, $root, 'field', $values['value'], array('field' => $field, 'delta' => $delta, 'langcode' => $language));
        }
      }
      elseif (!empty($field_values) && is_string($field_values)) { // Test this without field translation
        insert_child_element($dom, $root, 'field', $field_values, array('field' => $field));
      }
    }
  }

  $xml = $dom->saveXML();
  return $xml;
}

function cancel_fieldable_panels_submission($selected_submission) {
  $pd4 = get_project_director_details();
  $globalLink = new GlobalLink();
  $submission_name = get_fieldable_panels_submission_name($selected_submission);
  $globalLink->submissionName = $submission_name;
  $globalLink->submissionTicket = $selected_submission;

  cancel_ProjectDirector_submission($pd4, $globalLink);
  update_transperfect_fieldable_panels_submission($globalLink);
}

function check_tpt_fieldable_panels_status($ids_arr) {
  $status = TRUE;
  $query = db_select('transperfect_core_fieldable_panels', 'tcfp')
          ->fields('tcfp', array('id'))
          ->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $results = $query->execute();
  $rows = array();
  foreach ($results as $item) {
    $rows[$item->rid] = $item->rid;
  }

  foreach ($ids_arr as $val) {
    if (!in_array($val, $rows)) {
      unset($ids_arr[$val]);
      $status = FALSE;
    }
  }

  if (!$status) {
    drupal_set_message(t('Cannot cancel documents that have been cancelled in Globallink.'), 'warning', NULL);
  }

  return $ids_arr;
}

function clear_cancelled_fieldable_panels_documents() {
  $count = 0;
  $query = db_select('transperfect_core_fieldable_panels', 'tcfp')
          ->fields('tcfp', array('submission_ticket'))
          ->distinct()
          ->condition('status', TPT_STATUS_CANCELLED, '=');
  $results = $query->execute();
  foreach ($results as $item) {
    update_fieldable_panels_submission_status($item->submission_ticket, TPT_STATUS_PENDING_TRANSLATIONS);
    $count++;
  }

  return $count;
}

function get_fieldable_panels_submission_name($submission_ticket) {
  $query = db_select('transperfect_core_fieldable_panels', 'tcfp');
  $query->fields('tcfp');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $results = $query->execute();
  foreach ($results as $row) {
    if ($row->submission != '') {
      return $row->submission;
    }
  }
}

function update_transperfect_fieldable_panels_submission(&$globalLink) {
  db_update('transperfect_core_fieldable_panels')
          ->fields(array('status' => TPT_STATUS_PENDING_TRANSLATIONS, 'timestamp' => REQUEST_TIME))
          ->condition('submission_ticket', $globalLink->submissionTicket, '=')
          ->condition('submission', $globalLink->submissionName, '=')
          ->execute();
}

function get_fieldable_panels_pane_fields() {
  $fields = array();
  foreach (field_info_instances('fieldable_panels_pane', 'fieldable_panels_pane') as $field) {
    $fields[] = $field['field_name'];
  }
  return $fields;
}

function get_fieldable_panels_active_submission_names() {
  $query = db_select('transperfect_core_fieldable_panels', 'tcfp');
  $query->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $query->distinct();
  $query->fields('tcfp');
  $results = $query->execute();
  $arr = array('' => '-- Select a Submission --');
  foreach ($results as $row) {
    $arr[$row->submission_ticket] = $row->submission;
  }
  return $arr;
}

function get_active_fieldable_panels_submission_rows($fpid) {
  $query = db_select('transperfect_core_fieldable_panels', 'tcfp');
  $query->condition('status', TPT_STATUS_SENT_FOR_TRANSLATIONS, '=');
  $query->condition('fpid', $fpid, '=');
  $query->fields('tcfp');
  $results = $query->execute();
  $arr = array();
  foreach ($results as $row) {
    if (array_key_exists($row->fpid, $arr)) {
      array_push($arr[$row->fpid], $row);
    }
    else {
      $arr[$row->fpid] = array($row);
    }
  }
  $final_arr = array();
  foreach ($arr as $fpid => $mlid_arr) {
    $sub_arr = array();
    foreach ($mlid_arr as $r) {
      if (array_key_exists($r->submission, $sub_arr)) {
        array_push($sub_arr[$r->submission], $r->target);
      }
      else {
        $sub_arr[$r->submission] = array($r->target);
      }
    }
    if (count($sub_arr) > 0) {
      $final_arr[$fpid] = $sub_arr;
    }
  }
  if (count($final_arr) > 0) {
    return $final_arr;
  }
  return FALSE;
}

function get_distinct_active_fieldable_panels_submission_names() {
  $query = db_select('transperfect_core_fieldable_panels', 'tcfp');
  $query->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $query->distinct();
  $query->fields('tcfp');
  $results = $query->execute();
  $arr = array('[any]' => t(' Any '));
  foreach ($results as $row) {
    $arr[$row->submission] = $row->submission;
  }
  return $arr;
}

function get_fieldable_panels_submission_status() {
  $query = db_select('transperfect_core_fieldable_panels', 'tcfp');
  $query->fields('tcfp', array('submission_ticket'));
  $query->distinct();
  $query->condition('status', TPT_STATUS_SENT_FOR_TRANSLATIONS, '=');
  $results = $query->execute();

  foreach ($results as $row) {
    if ($row->submission_ticket) {
      try {
        $pd4 = get_project_director_details();
        $status = get_status($pd4, $row->submission_ticket);
        if (!$status || $status == PD_STATUS_CANCELLED) {
          update_fieldable_panels_submission_status($row->submission_ticket);
        }
      }
      catch (SoapFault $se) {
        update_fieldable_panels_submission_status($row->submission_ticket);
      }
      catch (Exception $ex) {
        update_fieldable_panels_submission_status($row->submission_ticket);
      }
    }
  }
}

function get_sent_tpt_fieldable_panels_rows($submission_ticket, $document_ticket, $target_locale) {
  $query = db_select('transperfect_core_fieldable_panels', 'tcfp');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $query->condition('document_ticket', $document_ticket, '=');
  $query->condition('target', $target_locale, '=');
  $query->fields('tcfp');
  $results = $query->execute();
  foreach ($results as $row) {
    return $row;
  }

  return FALSE;
}

function get_fieldable_panels_translation_status($fpid, $tgt_locale, &$title) {
  $query = db_select('transperfect_core_fieldable_panels', 'tcfp');
  $query->condition('fpid', $fpid, '=');
  $query->condition('target', $tgt_locale, '=');
  $query->distinct();
  $query->fields('tcfp');
  $results = $query->execute();
  foreach ($results as $row) {
    $title = $row->title;
    $entity = fieldable_panels_panes_load($fpid);
    if (empty($entity)) {
      return TPT_STATUS_SOURCE_DELETED;
    }
    elseif ($row->status == TPT_STATUS_SENT_FOR_TRANSLATIONS) {
      return TPT_STATUS_COMPLETED;
    }
    elseif ($row->status == TPT_STATUS_ERROR) {
      return TPT_STATUS_ERROR;
    }
  }

  return TPT_STATUS_SENT_FOR_TRANSLATIONS;
}

function get_tpt_fieldable_panels_row_id_from_submission($submission_ticket, $document_ticket, $target_locale) {
  $query = db_select('transperfect_core_fieldable_panels', 'tcfp');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $query->condition('document_ticket', $document_ticket, '=');
  $query->condition('target', $target_locale, '=');
  $query->fields('tcfp');
  $results = $query->execute();
  foreach ($results as $row) {
    return $row->id;
  }
  return FALSE;
}

function get_sent_tpt_fieldable_panels_rows_by_fpid($fpid) {
  $result = db_select('transperfect_core_fieldable_panels', 'tcfp')
          ->fields('tcfp')
          ->condition('fpid', $fpid, '=')
          ->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN')
          ->execute();
  $rows = array();
  foreach ($result as $row) {
    $rows[] = $row;
  }
  return $rows;
}

function get_transperfect_fieldable_panels_row($fpid, $source, $target) {
  $result = db_select('transperfect_core_fieldable_panels', 'tcfp')
          ->fields('tcfp')
          ->condition('fpid', $fpid, '=')
          ->condition('source', $source, '=')
          ->condition('target', $target, '=')
          ->execute();
  foreach ($result as $row) {
    return $row;
  }

  return FALSE;
}

function get_tp_fieldable_panels_row_by_id($id) {
  $result = db_select('transperfect_core_fieldable_panels', 'tcfp')
          ->fields('tcfp')
          ->condition('id', $id, '=')
          ->execute();
  foreach ($result as $row) {
    return $row;
  }

  return FALSE;
}

function get_translated_fieldable_panels_array($xml) {
  if (is_null($xml) || !is_string($xml) || $xml == '') {
    return array();
  }

  $dom = new DomDocument;
  $dom->preserveWhiteSpace = FALSE;
  $dom->loadXML($xml);

  $arr = array();

  $titles = $dom->getElementsByTagName('title');
  foreach ($titles as $title) {
    $arr['title'] = $title->nodeValue;
  }

  $contents = $dom->getElementsByTagName('content');

  $fpid = '';
  foreach ($contents as $content) {
    if (!is_null($content->attributes)) {
      foreach ($content->attributes as $attrName => $attrNode) {
        if ('fpid' == $attrName) {
          $fpid = $attrNode->value;
        }
      }
    }
  }

  if ($fpid == '') {
    return array();
  }

  $arr['fpid'] = $fpid;

  $fields = $dom->getElementsByTagName('field');
  foreach ($fields as $field) {
    $fieldObject = new GLField();
    $fieldObject->type = 'field';
    $fieldObject->translatedContent = $field->nodeValue;

    if (!is_null($field->attributes)) {
      foreach ($field->attributes as $attrName => $attrNode) {
        switch ($attrName) {
          case 'field':
            $fieldObject->fieldName = $attrNode->value;
            continue 2;
          case 'delta':
            $fieldObject->delta = $attrNode->value;
            continue 2;
          case 'langcode':
            $fieldObject->langcode = $attrNode->value;
            continue 2;
        }
      }
      if (is_null($fieldObject->langcode)) {
        $fieldObject->langcode = LANGUAGE_NONE;
      }
      if (is_null($fieldObject->delta)) {
        $fieldObject->delta = '0';
      }
      $arr[$fieldObject->fieldName][$fieldObject->langcode][$fieldObject->delta] = $fieldObject;
    }
  }

  return $arr;
}

function update_fieldable_panels_submission_status($submission_ticket, $status = TPT_STATUS_CANCELLED) {
  db_update('transperfect_core_fieldable_panels')
          ->fields(array(
              'status' => $status,
              'timestamp' => REQUEST_TIME
                  )
          )
          ->condition('submission_ticket', $submission_ticket, '=')
          ->execute();
}

function get_translated_fieldable_panels($pd4, &$globalLink_arr) {
  $count = 0;

  foreach ($globalLink_arr as $globalLink) {
    $targetXML = downloadTargetResource($pd4, $globalLink->targetTicket);
    if ($globalLink->sourceDeleted || empty($targetXML)) {
      continue;
    }

    $count++;
    $target_locale = get_drupal_locale_code($globalLink->targetLocale);
    $source_locale = get_drupal_locale_code($globalLink->sourceLocale);

    $translated_arr = get_translated_fieldable_panels_array($targetXML);
    try {
      $fpid = $translated_arr['fpid'];
      unset($translated_arr['fpid']);
      $entity = fieldable_panels_panes_load($fpid);

      if (empty($entity)) {
        update_tpt_fieldable_panels_status($globalLink, TPT_STATUS_SOURCE_DELETED);
        continue;
      }

      $fields = get_fieldable_panels_pane_fields();
      foreach ($fields as $field) {
        $t_field_lang = LANGUAGE_NONE;
        if (empty($translated_arr[$field])) {
          continue;
        }

        if (key($translated_arr[$field]) !== LANGUAGE_NONE) {
          $t_field_lang = key($translated_arr[$field]);
        }
   
        if (empty($entity->{$field}[$target_locale]) && $target_locale != LANGUAGE_NONE) {
            $entity->{$field}[$target_locale] = $entity->{$field}[$t_field_lang];
        }

        $t_field_arr = $translated_arr[$field][$source_locale];
        foreach ($entity->{$field}[$target_locale] as $delta => $fp_field) {
          if (empty($t_field_arr[$delta])) {
            continue;
          }

          $translation = $t_field_arr[$delta]->translatedContent;
          $entity->{$field}[$target_locale][$delta]['value'] = $translation;
        }
      }

      $entity->revision = 1;
      $entity->translations->data[$target_locale] = array(
        'language' => $target_locale, // Target language
        'source' => $source_locale, // Source language
        'uid' => $entity->uid,
        'status' => variable_get('transperfect_publish_node', 0), // publish translation
        'translate' => 0, // Translation out of date
        'created' => $entity->created,
      );

      $entity->translations->hook[$target_locale] = array(
        'hook' => 'insert',
        'date' => NULL,
      );

      fieldable_panels_panes_save($entity);
      update_tpt_fieldable_panels_status($globalLink);
      sendDownloadConfirmation($globalLink->targetTicket, $pd4);
    }
    catch (SoapFault $se) {
      $count--;
      update_tpt_fieldable_panels_status($globalLink, TPT_STATUS_ERROR);
      gl_log(TPT_OBJECT_TYPE_MENU, TPT_LOGGING_SEVERITY_ERROR, "SOAP Exception in Sending Download Confirmation - update_deleted_menu_records - Code[$se->faultcode], Message[$se->faultstring]");
      form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
    }
    catch (Exception $e) {
      $count--;
      update_tpt_fieldable_panels_status($globalLink, TPT_STATUS_ERROR);
      gl_log(TPT_OBJECT_TYPE_MENU, TPT_LOGGING_SEVERITY_ERROR, "Exception in Saving Menu Object - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
      form_set_error('', check_plain('Error: ' . $e->getMessage()));
    }
  }
  return $count;
}

function update_tpt_fieldable_panels_status(&$globalLink, $status = TPT_STATUS_PENDING_TRANSLATIONS) {
  db_update('transperfect_core_fieldable_panels')
          ->fields(array('status' => $status, 'timestamp' => REQUEST_TIME))
          ->condition('submission', $globalLink->submissionName, '=')
          ->condition('document_ticket', $globalLink->documentTicket, '=')
          ->condition('source', $globalLink->sourceLocale, '=')
          ->condition('target', $globalLink->targetLocale, '=')
          ->execute();
}

function update_deleted_fieldable_panels_records($pd4, $globalLink) {
  try {
    $globalLink->status = TPT_STATUS_SOURCE_DELETED;
    sendDownloadConfirmation($globalLink->targetTicket, $pd4);
    update_tpt_fieldable_panels_status($globalLink, TPT_STATUS_SOURCE_DELETED);
  }
  catch (SoapFault $se) {
    gl_log(TPT_OBJECT_TYPE_FIELDABLE_PANELS, TPT_LOGGING_SEVERITY_ERROR, "SOAP Exception in Sending Download Confirmation - update_deleted_menu_records - Code[$se->faultcode], Message[$se->faultstring]");
    form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
    return FALSE;
  }
  catch (Exception $e) {
    gl_log(TPT_OBJECT_TYPE_FIELDABLE_PANELS, TPT_LOGGING_SEVERITY_ERROR, "Exception in update_deleted_menu_records - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
    form_set_error('', check_plain('Error: ' . $e->getMessage()));
    return FALSE;
  }

  return TRUE;
}

function update_fieldable_panels_ticket_id($arr, $project_code) {
  foreach ($arr as $globalLink) {
    $target_locale_arr = $globalLink->targetLocale;
    $type = $globalLink->type;
    if ($type == 'fieldable_panels_panes') {
      $fpid = $globalLink->otherObjectId;
      $fieldable_panels = fieldable_panels_panes_load($fpid);
      foreach ($target_locale_arr as $target_locale) {
        $row = get_transperfect_fieldable_panels_row($fpid, $globalLink->sourceLocale, $target_locale);
        $title = 'Title Not Found';
        if (!empty($fieldable_panels->title)) {
          $title = $fieldable_panels->title;
        }
        else {
          if (!empty($fieldable_panels->admin_title)) {
            $title = $fieldable_panels->admin_title;
          }
        }
        if ($row) {
          db_update('transperfect_core_fieldable_panels')
              ->fields(array('title' => $title, 'document_ticket' => $globalLink->documentTicket,
                  'submission' => $globalLink->submissionName, 'submission_ticket' => $globalLink->submissionTicket,
                  'status' => TPT_STATUS_SENT_FOR_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'last_modified' => REQUEST_TIME, 'project_code' => $project_code))
              ->condition('id', $row->id, '=')
              ->execute();
        }
        else {
          db_insert('transperfect_core_fieldable_panels')
              ->fields(array('fpid' => $fpid, 'title' => $title,
                  'source' => $globalLink->sourceLocale, 'target' => $target_locale, 'document_ticket' => $globalLink->documentTicket,
                  'submission' => $globalLink->submissionName, 'submission_ticket' => $globalLink->submissionTicket,
                  'status' => TPT_STATUS_SENT_FOR_TRANSLATIONS, 'timestamp' => REQUEST_TIME, 'last_modified' => REQUEST_TIME, 'project_code' => $project_code))
              ->execute();
        }
      }
    }
  }
}