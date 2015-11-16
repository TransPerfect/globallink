<?php

/**
 * @file
 * Miscellaneous TransPerfect functions for menu translations.
 */

require_once TPT_ROOT . '/' . ('transperfect_settings.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_common.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_receive_translations.inc');

function transperfect_send_menus_for_translations($mlids, $pd4, $submission_name, $due_date, $project_code, $source_locale, $target_locale_arr, $submission_details) {
  $submitter = $submission_details['submitter'];
  $globallink_arr = array();

  foreach ($mlids as $mlid) {
    $rows = transperfect_get_sent_menu_rows_by_mlid($mlid);
    $target_arr = $target_locale_arr;

    foreach ($rows as $row) {
      if (array_search($row->target, $target_locale_arr)) {
        unset($target_arr[$row->target]);
      }
    }

    if (empty($target_arr)) {
      continue;
    }

    $xml = get_menu_xml($mlid);
    $name = 'Menu_' . $mlid . '.xml';
    watchdog(TPT_OBJECT_TYPE_GLOBALLINK, 'XML - %xml', array('%xml' => $xml), WATCHDOG_DEBUG);

    $globallink = new GlobalLink();
    $globallink->type = 'menu';
    $globallink->metadata = 'menu';
    $globallink->sourceLocale = $source_locale;
    $globallink->targetLocale = $target_arr;
    $globallink->sourceXML = $xml;
    $globallink->sourceFileName = $name;
    $globallink->submissionName = $submission_name;
    $globallink->dueDate = $due_date;
    $globallink->otherObjectId = $mlid;
    $globallink->submissionInstructions = $submission_details['instructions'] . "\nSubmitter: " . $submitter;
    $globallink_arr[] = $globallink;
  }

  if (!empty($globallink_arr)) {
    transperfect_send_documents_for_translation_to_pd($globallink_arr, $pd4, $project_code, $submitter);
  }

  return $globallink_arr;
}

function get_menu_xml($mlid) {
  $dom = new DOMDocument('1.0', 'UTF-8');
  $dom->formatOutput = TRUE;

  $root = $dom->createElement('content');
  $dom->appendChild($root);

  $id = $dom->createAttribute('mlid');
  $id->value = $mlid;
  $root->appendChild($id);

  $link = menu_link_load($mlid);
  transperfect_insert_child_element($dom, $root, 'menu', $link['link_title'], array('name' => 'link_title'));

  if (isset($link['options']) && isset($link['options']['attributes'])) {
    if (isset($link['options']['attributes']['title'])) {
      if (is_string($link['options']['attributes']['title'])) {
        transperfect_insert_child_element($dom, $root, 'menu', $link['options']['attributes']['title'], array('name' => 'title'));
      }
    }
  }

  $xml = $dom->saveXML();

  return $xml;
}

function transperfect_update_menu_ticket_id($arr, $project_code) {
  foreach ($arr as $globallink) {
    $target_locale_arr = $globallink->targetLocale;
    $type = $globallink->type;

    if ($type != 'menu') {
      continue;
    }

    $mlid = $globallink->otherObjectId;
    $link = menu_link_load($mlid);

    foreach ($target_locale_arr as $target_locale) {
      $row = transperfect_get_menu_row($mlid, $type, $globallink->sourceLocale, $target_locale);

      if ($row) {
        db_update('transperfect_core_menu')
        ->fields(array(
          'title' => $link['link_title'],
          'document_ticket' => $globallink->documentTicket,
          'submission' => $globallink->submissionName,
          'submission_ticket' => $globallink->submissionTicket,
          'status' => TPT_STATUS_SENT_FOR_TRANSLATIONS,
          'timestamp' => REQUEST_TIME,
          'last_modified' => REQUEST_TIME,
          'project_code' => $project_code,
        ))
        ->condition('rid', $row->rid, '=')
        ->execute();
      }
      else {
        db_insert('transperfect_core_menu')
        ->fields(array(
          'object_id' => $mlid,
          'object_type' => $globallink->type,
          'title' => $link['link_title'],
          'source' => $globallink->sourceLocale,
          'target' => $target_locale,
          'document_ticket' => $globallink->documentTicket,
          'submission' => $globallink->submissionName,
          'submission_ticket' => $globallink->submissionTicket,
          'status' => TPT_STATUS_SENT_FOR_TRANSLATIONS,
          'timestamp' => REQUEST_TIME,
          'last_modified' => REQUEST_TIME,
          'project_code' => $project_code,
        ))
        ->execute();
      }
    }
  }
}

function transperfect_get_menu_row($object_id, $object_type, $source, $target) {
  $result = db_select('transperfect_core_menu', 'tco')
  ->fields('tco')
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

function transperfect_update_deleted_menu_records($pd4, $globallink) {
  try {
    $globallink->status = TPT_STATUS_SOURCE_DELETED;

    transperfect_send_download_confirmation($globallink->targetTicket, $pd4);
    transperfect_update_menu_status($globallink, TPT_STATUS_SOURCE_DELETED);
  }
  catch (SoapFault $se) {
    watchdog(TPT_OBJECT_TYPE_GLOBALLINK, 'SOAP Exception - %function - Code[%faultcode], Message[%faultstring]', array('%function' => __FUNCTION__, '%faultcode' => $se->faultcode, '%faultstring' => $se->faultstring), WATCHDOG_ERROR);
    form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));

    return FALSE;
  }
  catch (Exception $e) {
    watchdog(TPT_OBJECT_TYPE_GLOBALLINK, 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
    form_set_error('', check_plain('Error: ' . $e->getMessage()));

    return FALSE;
  }

  return TRUE;
}

function transperfect_get_menu_translation_status($mlid, $tgt_locale, &$title) {
  $query = db_select('transperfect_core_menu', 'tco');
  $query->condition('object_id', $mlid, '=');
  $query->condition('target', $tgt_locale, '=');
  $query->distinct();
  $query->fields('tco');

  $results = $query->execute();

  foreach ($results as $row) {
    $title = $row->title;
    $s_link = menu_link_load($mlid);

    if (!$s_link) {
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

function transperfect_get_menu_active_submission_names() {
  $query = db_select('transperfect_core_menu', 'tco');
  $query->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $query->distinct();
  $query->fields('tco');

  $results = $query->execute();

  $arr = array('' => '-- Select a Submission --');

  foreach ($results as $row) {
    $arr[$row->submission_ticket] = $row->submission;
  }

  return $arr;
}

function transperfect_cancel_menu_records($rowids, $pd4) {
  $globallink_arr = array();

  foreach ($rowids as $rid) {
    $row = transperfect_get_other_row($rid, 'menu');

    $globallink = new GlobalLink();
    $globallink->tptRowId = $row->rid;
    $globallink->targetLocale = $row->target;
    $globallink->documentTicket = $row->document_ticket;
    $globallink->submissionTicket = $row->submission_ticket;
    $globallink_arr[$rid] = $globallink;
  }

  transperfect_cancel_select_documents($pd4, $globallink_arr);
  transperfect_update_menu_row_document($globallink_arr);
}

function transperfect_update_menu_row_document(&$globallink_arr) {
  foreach ($globallink_arr as $globallink) {
    if ($globallink->cancelled) {
      db_update('transperfect_core_menu')
      ->fields(array(
        'status' => TPT_STATUS_PENDING_TRANSLATIONS,
        'timestamp' => REQUEST_TIME,
      ))
      ->condition('rid', $globallink->tptRowId, '=')
      ->execute();
    }
  }
}

function transperfect_cancel_menu_submission($selected_submission) {
  $pd4 = transperfect_get_project_director_details();

  $globallink = new GlobalLink();

  $submission_name = transperfect_get_menu_submission_name($selected_submission);
  $globallink->submissionName = $submission_name;
  $globallink->submissionTicket = $selected_submission;

  transperfect_cancel_pd_submission($pd4, $globallink);
  transperfect_update_menu_submission($globallink);
}

function transperfect_get_menu_submission_name($submission_ticket) {
  $query = db_select('transperfect_core_menu', 'tc');
  $query->fields('tc');
  $query->condition('submission_ticket', $submission_ticket, '=');

  $results = $query->execute();

  foreach ($results as $row) {
    if ($row->submission != '') {
      return $row->submission;
    }
  }
}

function transperfect_update_menu_submission(&$globallink) {
  db_update('transperfect_core_menu')
  ->fields(array(
    'status' => TPT_STATUS_PENDING_TRANSLATIONS,
    'timestamp' => REQUEST_TIME,
  ))
  ->condition('submission_ticket', $globallink->submissionTicket, '=')
  ->condition('submission', $globallink->submissionName, '=')
  ->execute();
}

function transperfect_get_i18n_menu_set($tsid) {
  $arr = array();

  $query = db_select('menu_links', 'ml');
  $query->fields('ml');
  $query->condition('i18n_tsid', $tsid, '=');

  $results = $query->execute();

  foreach ($results as $row) {
    $arr[$row->language] = $row;
  }

  return $arr;
}

function transperfect_sort_gl_objects_by_menu_order($gl_1, $gl_2) {
  $link_1 = menu_link_load($gl_1->otherObjectId);
  $link_2 = menu_link_load($gl_2->otherObjectId);

  return (transperfect_count_parents($link_1) < transperfect_count_parents($link_2)) ? -1 : 1;
}

function transperfect_count_parents($menu_link) {
  $count = 0;

  if (isset($menu_link)) {
    if (isset($menu_link['p1']) && $menu_link['p1'] != 0) {
      $count++;
    }

    if (isset($menu_link['p2']) && $menu_link['p2'] != 0) {
      $count++;
    }

    if (isset($menu_link['p3']) && $menu_link['p3'] != 0) {
      $count++;
    }

    if (isset($menu_link['p4']) && $menu_link['p4'] != 0) {
      $count++;
    }

    if (isset($menu_link['p5']) && $menu_link['p5'] != 0) {
      $count++;
    }

    if (isset($menu_link['p6']) && $menu_link['p6'] != 0) {
      $count++;
    }

    if (isset($menu_link['p7']) && $menu_link['p7'] != 0) {
      $count++;
    }

    if (isset($menu_link['p8']) && $menu_link['p8'] != 0) {
      $count++;
    }

    if (isset($menu_link['p9']) && $menu_link['p9'] != 0) {
      $count++;
    }
  }

  return $count;
}

function transperfect_get_translated_menus($pd4, &$globallink_arr) {
  module_load_include('inc', 'i18n_menu', 'i18n_menu.admin');

  $count = 0;
  $source_link = array('link_title' => 'No Title');

  usort($globallink_arr, 'transperfect_sort_gl_objects_by_menu_order');

  foreach ($globallink_arr as $globallink) {
    $target_xml = transperfect_download_target_resource($pd4, $globallink->targetTicket);

    if ($globallink->sourceDeleted) {
      continue;
    }

    if (!isset($target_xml)) {
      continue;
    }

    $count++;

    $language = transperfect_get_drupal_locale_code($globallink->targetLocale);
    $arr = transperfect_get_translated_menu_items($target_xml);

    try {
      $mlid = $arr['mlid'];
      $t_link_title = $arr['link_title'];
      $source_link = menu_link_load($mlid);

      if ($source_link) {
        $t_title = FALSE;

        if (isset($source_link['options'])) {
          if (isset($source_link['options']['attributes'])) {
            if (isset($source_link['options']['attributes']['title'])) {
              if ($source_link['options']['attributes']['title'] != '') {
                $t_title = isset($arr['title']) ? $arr['title'] : FALSE;
              }
            }
          }
        }

        $t_plid = 0;

        if (isset($source_link['plid']) && $source_link['plid'] != 0) {
          $plid = $source_link['plid'];
          $p_source_link = menu_link_load($plid);
          $p_path_arr = translation_path_get_translations($p_source_link['link_path']);

          if (isset($p_path_arr[$language])) {
            $p_target_link = i18n_menu_link_load($p_path_arr[$language], $language);
            $t_plid = $p_target_link['mlid'];
          }
          elseif (isset($p_source_link['i18n_tsid']) && $p_source_link['i18n_tsid'] != 0) {
            $i18n_arr = transperfect_get_i18n_menu_set($p_source_link['i18n_tsid']);

            if (isset($i18n_arr[$language])) {
              $menu_link_row = $i18n_arr[$language];
              $t_plid = $menu_link_row->mlid;
            }
          }
        }

        $path_arr = translation_path_get_translations($source_link['link_path']);
        $link_path = '';

        if (!isset($path_arr[$language])) {
          $link_path = $source_link['link_path'];
        }
        else {
          $link_path = $path_arr[$language];
        }

        $t_link = NULL;

        if (isset($source_link['i18n_tsid']) && $source_link['i18n_tsid'] != 0) {
          $i18n_arr = transperfect_get_i18n_menu_set($source_link['i18n_tsid']);

          if (isset($i18n_arr[$language])) {
            $menu_link_row = $i18n_arr[$language];
            $t_link = menu_link_load($menu_link_row->mlid);
          }
        }

        if (is_array($t_link)) {
          $t_link['link_title'] = $t_link_title;
          $t_link['menu_name'] = $source_link['menu_name'];
          $t_link['customized'] = $source_link['customized'];
          $t_link['has_children'] = $source_link['has_children'];
          $t_link['expanded'] = $source_link['expanded'];
          $t_link['weight'] = $source_link['weight'];
          $t_link['hidden'] = $source_link['hidden'];

          if ($t_title && $t_link != '') {
            if (isset($t_link['options'])) {
              if (isset($t_link['options']['attributes'])) {
                $t_link['options']['attributes']['title'] = $t_title;
              }
              else {
                $t_link['options']['attributes'] = array('title' => $t_title);
              }
            }
          }
        }
        else {
          $t_link = array();

          $t_link['link_title'] = $t_link_title;
          $t_link['language'] = $language;
          $t_link['link_path'] = $link_path;
          $t_link['menu_name'] = $source_link['menu_name'];
          $t_link['customized'] = $source_link['customized'];
          $t_link['has_children'] = $source_link['has_children'];
          $t_link['expanded'] = $source_link['expanded'];
          $t_link['weight'] = $source_link['weight'];
          $t_link['hidden'] = $source_link['hidden'];

          if ($t_plid != 0) {
            $t_link['plid'] = $t_plid;
          }

          if ($t_title) {
            $t_link['options'] = array(
              'attributes' => array('title' => $t_title),
              'langcode' => $language,
            );
          }
        }

        menu_link_save($t_link);

        $translation_set = ($source_link['i18n_tsid']) ? i18n_translation_set_load($source_link['i18n_tsid']) : i18n_translation_set_create('menu_link');
        $translation_set->add_item($source_link, $source_link['language']);
        $translation_set->add_item($t_link, $language);
        $translation_set->save(TRUE);

        transperfect_update_menu_status($globallink);
      }
      else {
        // Source Link is deleted
        transperfect_update_menu_status($globallink, TPT_STATUS_SOURCE_DELETED);
      }

      transperfect_send_download_confirmation($globallink->targetTicket, $pd4);
    }
    catch (SoapFault $se) {
      $count--;

      transperfect_update_menu_status($globallink, TPT_STATUS_ERROR);
      watchdog(TPT_OBJECT_TYPE_GLOBALLINK, 'SOAP Exception - %function - Code[%faultcode], Message[%faultstring]', array('%function' => __FUNCTION__, '%faultcode' => $se->faultcode, '%faultstring' => $se->faultstring), WATCHDOG_ERROR);
      form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
    }
    catch (Exception $e) {
      $count--;

      transperfect_update_menu_status($globallink, TPT_STATUS_ERROR);
      watchdog(TPT_OBJECT_TYPE_GLOBALLINK, 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
      form_set_error('', check_plain('Error: ' . $e->getMessage()));
    }
  }

  return $count;
}

function transperfect_get_translated_menu_items($xml) {
  if (is_null($xml) || !is_string($xml) || $xml == '') {
    return array();
  }

  $dom = new DomDocument;
  $dom->preserveWhiteSpace = FALSE;
  $dom->loadXML($xml);

  $arr = array();

  $contents = $dom->getElementsByTagName('content');

  $mlid = '';

  foreach ($contents as $content) {
    if (is_null($content->attributes)) {
      continue;
    }

    foreach ($content->attributes as $attr_name => $attr_node) {
      if ($attr_name == 'mlid') {
        $mlid = $attr_node->value;
      }
    }
  }

  if ($mlid == '') {
    return array();
  }

  $arr['mlid'] = $mlid;

  $menus = $dom->getElementsByTagName('menu');

  foreach ($menus as $menu) {
    $type = '';

    foreach ($menu->attributes as $attr_name => $attr_node) {
      if ($attr_name == 'name') {
        $type = $attr_node->value;
      }
    }

    $arr[$type] = $menu->nodeValue;
  }

  return $arr;
}

function transperfect_update_menu_status(&$globallink, $status = TPT_STATUS_PENDING_TRANSLATIONS) {
  db_update('transperfect_core_menu')
  ->fields(array('status' => $status, 'timestamp' => REQUEST_TIME))
  ->condition('submission', $globallink->submissionName, '=')
  ->condition('document_ticket', $globallink->documentTicket, '=')
  ->condition('source', $globallink->sourceLocale, '=')
  ->condition('target', $globallink->targetLocale, '=')
  ->execute();
}

function transperfect_get_active_menu_submission_rows($object_id) {
  $query = db_select('transperfect_core_menu', 'tc');
  $query->condition('status', TPT_STATUS_SENT_FOR_TRANSLATIONS, '=');
  $query->condition('object_id', $object_id, '=');
  $query->fields('tc');

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

  foreach ($arr as $mlid => $mlid_arr) {
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
      $final_arr[$mlid] = $sub_arr;
    }
  }

  if (count($final_arr) > 0) {
    return $final_arr;
  }

  return FALSE;
}

function transperfect_get_sent_menu_rows_by_mlid($mlid) {
  $result = db_select('transperfect_core_menu', 'tc')
  ->fields('tc')
  ->condition('object_id', $mlid, '=')
  ->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN')
  ->execute();

  $rows = array();

  foreach ($result as $row) {
    $rows[] = $row;
  }

  return $rows;
}

function transperfect_get_sent_menu_rows($submission_ticket, $document_ticket, $target_locale) {
  $query = db_select('transperfect_core_menu', 'tc');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $query->condition('document_ticket', $document_ticket, '=');
  $query->condition('target', $target_locale, '=');
  $query->fields('tc');

  $results = $query->execute();

  foreach ($results as $row) {
    return $row;
  }

  return FALSE;
}

function transperfect_get_menu_row_id_from_submission($submission_ticket, $document_ticket, $target_locale) {
  $query = db_select('transperfect_core_menu', 'tc');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $query->condition('document_ticket', $document_ticket, '=');
  $query->condition('target', $target_locale, '=');
  $query->fields('tc');

  $results = $query->execute();

  foreach ($results as $row) {
    return $row->rid;
  }

  return FALSE;
}

function transperfect_load_menu_data($mlid) {
  $query = db_select('menu_links', 'ml')
  ->fields('ml')
  ->condition('mlid', $mlid, '=');

  $results = $query->execute();

  $rows = array();

  foreach ($results as $item) {
    $rows[] = $item;
  }

  return $rows;
}

function transperfect_get_distinct_active_menu_submission_names() {
  $query = db_select('transperfect_core_menu', 'tc');
  $query->condition('status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN');
  $query->distinct();
  $query->fields('tc');

  $results = $query->execute();

  $arr = array('[any]' => t('Any'));

  foreach ($results as $row) {
    $arr[$row->submission] = $row->submission;
  }

  return $arr;
}

function transperfect_update_menu_submission_status($submission_ticket, $status = TPT_STATUS_CANCELLED) {
  db_update('transperfect_core_menu')
  ->fields(array('status' => $status, 'timestamp' => REQUEST_TIME))
  ->condition('submission_ticket', $submission_ticket, '=')
  ->execute();
}

function transperfect_get_menu_submission_status() {
  $query = db_select('transperfect_core_menu', 'tc');
  $query->fields('tc', array('submission_ticket'));
  $query->distinct();
  $query->condition('status', TPT_STATUS_SENT_FOR_TRANSLATIONS, '=');

  $results = $query->execute();

  foreach ($results as $row) {
    if ($row->submission_ticket) {
      try {
        $pd4 = transperfect_get_project_director_details();

        $status = transperfect_get_status($pd4, $row->submission_ticket);

        if (!$status || $status == PD_STATUS_CANCELLED) {
          transperfect_update_menu_submission_status($row->submission_ticket);
        }
      }
      catch (SoapFault $se) {
        transperfect_update_menu_submission_status($row->submission_ticket);
      }
      catch (Exception $ex) {
        transperfect_update_menu_submission_status($row->submission_ticket);
      }
    }
  }
}

function transperfect_check_menu_status($rids_arr) {
  $status = TRUE;

  $query = db_select('transperfect_core_menu', 'tc')
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

function transperfect_clear_cancelled_menu_documents() {
  $count = 0;

  $query = db_select('transperfect_core_menu', 'tc')
  ->fields('tc', array('submission_ticket'))
  ->distinct()
  ->condition('status', TPT_STATUS_CANCELLED, '=');

  $results = $query->execute();

  foreach ($results as $item) {
    transperfect_update_menu_submission_status($item->submission_ticket, TPT_STATUS_PENDING_TRANSLATIONS);

    $count++;
  }

  return $count;
}
