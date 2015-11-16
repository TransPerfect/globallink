<?php

/**
 * @file
 * Miscellaneous functions for TransPerfect taxonomy translations.
 */

require_once TPT_ROOT . '/' . ('transperfect_settings.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_common.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_receive_translations.inc');

function transperfect_send_taxonomies_for_translations($lids, $pd4, $submission_name, $due_date, $project_code, $source_locale, $target_locale_arr, $submission_details) {
  $submitter = $submission_details['submitter'];
  $globallink_arr = array();

  foreach ($lids as $lid) {
    $rows = transperfect_get_sent_taxonomy_rows_by_bid($lid);
    $target_arr = $target_locale_arr;

    foreach ($rows as $row) {
      if (array_search($row->target, $target_locale_arr)) {
        unset($target_arr[$row->target]);
      }
    }

    if (empty($target_arr)) {
      continue;
    }

    $term = taxonomy_term_load($lid);
    $path = drupal_lookup_path('alias', 'taxonomy/term/' . $lid, transperfect_get_drupal_locale_code($source_locale));

    if (empty($term->name)) {
      continue;
    }

    $xml = transperfect_get_taxonomy_xml($term, $path);

    $name = 'Taxonomy_' . $lid . '.xml';

    $globallink = new GlobalLink();
    $globallink->type = 'taxonomy';
    $globallink->metadata = 'taxonomy';
    $globallink->sourceLocale = $source_locale;
    $globallink->targetLocale = $target_arr;
    $globallink->sourceXML = $xml;
    $globallink->sourceFileName = $name;
    $globallink->submissionName = $submission_name;
    $globallink->dueDate = $due_date;
    $globallink->otherObjectId = $lid;
    $globallink->submissionInstructions = $submission_details['instructions'] . "\nSubmitter: " . $submitter;
    $globallink_arr[] = $globallink;
  }

  if (!empty($globallink_arr)) {
    transperfect_send_documents_for_translation_to_pd($globallink_arr, $pd4, $project_code, $submitter);
  }

  return $globallink_arr;
}

function transperfect_get_taxonomy_xml($term, $path) {
  $dom = new DOMDocument('1.0', 'UTF-8');
  $dom->formatOutput = TRUE;

  $root = $dom->createElement('content');
  $dom->appendChild($root);

  $id = $dom->createAttribute('bid');
  $id->value = $term->tid;
  $root->appendChild($id);

  if (!empty($term->name)) {
    transperfect_insert_child_element($dom, $root, 'taxonomy', $term->name, array('name' => 'name'));
  }

  if (!empty($term->description)) {
    transperfect_insert_child_element($dom, $root, 'taxonomy', $term->description, array('name' => 'description'));
  }

  if (!empty($path)) {
    transperfect_insert_child_element($dom, $root, 'taxonomy', $path, array('name' => 'path'));
  }

  $xml = $dom->saveXML();

  return $xml;
}

function transperfect_get_taxonomy_strings($language = NULL, $group = 'taxonomy') {
  if (isset($language)) {
    $result = db_query('SELECT s.lid, s.source, s.context, s.location, t.plid, t.plural FROM {locales_source} s LEFT JOIN {locales_target} t ON s.lid = t.lid AND t.language = :language WHERE s.textgroup = :textgroup ORDER BY t.plid, t.plural', array(
      ':language' => $language,
      ':textgroup' => $group,
    ));
  }
  else {
    $result = db_query('SELECT s.lid, s.source, s.context, s.location, t.plid, t.plural FROM {locales_source} s LEFT JOIN {locales_target} t ON s.lid = t.lid WHERE s.textgroup = :textgroup ORDER BY t.plid, t.plural', array(
      ':textgroup' => $group,
    ));
  }

  $strings = array();

  foreach ($result as $child) {
    $string = array(
      'lid' => $child->lid,
      'location' => $child->location,
      'source' => $child->source,
      'context' => $child->context,
    );

    $strings[$child->location] = $string;
  }

  return $strings;
}

function transperfect_update_taxonomy_ticket_id($arr, $project_code) {
  foreach ($arr as $globallink) {
    $target_locale_arr = $globallink->targetLocale;
    $type = $globallink->type;

    if ($type != 'taxonomy') {
      continue;
    }

    $bid = $globallink->otherObjectId;
    $taxonomy = taxonomy_term_load($bid);

    foreach ($target_locale_arr as $target_locale) {
      $row = transperfect_get_taxonomy_row($bid, $type, $globallink->sourceLocale, $target_locale);

      if ($row) {
        db_update('transperfect_core_taxonomy')
        ->fields(array(
          'title' => $taxonomy->name,
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
        db_insert('transperfect_core_taxonomy')
        ->fields(array(
          'object_id' => $bid,
          'object_type' => $globallink->type,
          'title' => $taxonomy->name,
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

function transperfect_get_taxonomy_row($object_id, $object_type, $source, $target) {
  $result = db_select('transperfect_core_taxonomy', 'tco')
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

function transperfect_update_deleted_taxonomy_records($pd4, $globallink) {
  try {
    $globallink->status = TPT_STATUS_SOURCE_DELETED;

    transperfect_send_download_confirmation($globallink->targetTicket, $pd4);
    transperfect_update_taxonomy_status($globallink, TPT_STATUS_SOURCE_DELETED);
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

function transperfect_get_taxonomy_translation_status($bid, $tgt_locale, &$title) {
  $query = db_select('transperfect_core_taxonomy', 'tco');
  $query->condition('object_id', $bid, '=');
  $query->condition('target', $tgt_locale, '=');
  $query->distinct();
  $query->fields('tco');
  $results = $query->execute();

  foreach ($results as $row) {
    $title = $row->title;
    $s_taxonomy = taxonomy_term_load($bid);

    if (!$s_taxonomy || is_null($s_taxonomy)) {
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

function transperfect_get_taxonomy_active_submission_names() {
  $query = db_select('transperfect_core_taxonomy', 'tco');
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

function transperfect_cancel_taxonomy_records($rowids, $pd4) {
  $globallink_arr = array();

  foreach ($rowids as $rid) {
    $row = transperfect_get_other_row($rid, 'taxonomy');

    $globallink = new GlobalLink();
    $globallink->tptRowId = $row->rid;
    $globallink->targetLocale = $row->target;
    $globallink->documentTicket = $row->document_ticket;
    $globallink->submissionTicket = $row->submission_ticket;
    $globallink_arr[$rid] = $globallink;
  }

  transperfect_cancel_select_documents($pd4, $globallink_arr);
  transperfect_update_taxonomy_row_document($globallink_arr);
}

function transperfect_update_taxonomy_row_document(&$globallink_arr) {
  foreach ($globallink_arr as $globallink) {
    if ($globallink->cancelled) {
      db_update('transperfect_core_taxonomy')
      ->fields(array(
        'status' => TPT_STATUS_PENDING_TRANSLATIONS,
        'timestamp' => REQUEST_TIME,
      ))
      ->condition('rid', $globallink->tptRowId, '=')
      ->execute();
    }
  }
}

function transperfect_cancel_taxonomy_submission($selected_submission) {
  $pd4 = transperfect_get_project_director_details();

  $globallink = new GlobalLink();

  $submission_name = transperfect_get_taxonomy_submission_name($selected_submission);
  $globallink->submissionName = $submission_name;
  $globallink->submissionTicket = $selected_submission;

  transperfect_cancel_pd_submission($pd4, $globallink);
  transperfect_update_taxonomy_submission($globallink);
}

function transperfect_get_taxonomy_submission_name($submission_ticket) {
  $query = db_select('transperfect_core_taxonomy', 'tc');
  $query->fields('tc');
  $query->condition('submission_ticket', $submission_ticket, '=');
  $results = $query->execute();

  foreach ($results as $row) {
    if ($row->submission != '') {
      return $row->submission;
    }
  }
}

function transperfect_update_taxonomy_submission(&$globallink) {
  db_update('transperfect_core_taxonomy')
  ->fields(array(
    'status' => TPT_STATUS_PENDING_TRANSLATIONS,
    'timestamp' => REQUEST_TIME,
  ))
  ->condition('submission_ticket', $globallink->submissionTicket, '=')
  ->condition('submission', $globallink->submissionName, '=')
  ->execute();
}

function transperfect_get_translated_taxonomies($pd4, &$globallink_arr) {
  $count = 0;

  foreach ($globallink_arr as $globallink) {
    if ($globallink->sourceDeleted) {
      continue;
    }

    $target_xml = transperfect_download_target_resource($pd4, $globallink->targetTicket);

    if (!isset($target_xml)) {
      continue;
    }

    $count++;

    $language = transperfect_get_drupal_locale_code($globallink->targetLocale);
    $translated_arr = transperfect_get_translated_taxonomy_items($target_xml);

    try {
      $bid = $translated_arr['bid'];
      unset($translated_arr['bid']);

      $term = taxonomy_term_load($bid);

      if (empty($term)) {
        continue;
      }

      unset($term->tid);
      $term->language = $language;

      $new_term = taxonomy_term_save($term);

      if ($new_term != 1) {
        form_set_error('', check_plain('Unable to save new term for ' . $bid . ' where language is ' . $language));

        continue;
      }

      foreach ($translated_arr as $attribute => $translations) {
        switch ($attribute) {
          case 'name':
            $term->name = $translations['translation'];

            break;
          case 'description':
            $term->description = $translations['translation'];

            break;
          case 'path':
            $term->path['pathauto'] = FALSE;

            $uri = taxonomy_term_uri($term);
            $new_path = path_load($uri['path']);
            $new_path['alias'] = $translations['translation'];
            path_save($new_path);

            break;
        }
      }

      taxonomy_term_save($term);
      transperfect_update_taxonomy_status($globallink);
      transperfect_send_download_confirmation($globallink->targetTicket, $pd4);
    }
    catch (SoapFault $se) {
      $count--;

      transperfect_update_taxonomy_status($globallink, TPT_STATUS_ERROR);
      watchdog(TPT_OBJECT_TYPE_GLOBALLINK, 'SOAP Exception - %function - Code[%faultcode], Message[%faultstring]', array('%function' => __FUNCTION__, '%faultcode' => $se->faultcode, '%faultstring' => $se->faultstring), WATCHDOG_ERROR);
      form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
    }
    catch (Exception $e) {
      $count--;

      transperfect_update_taxonomy_status($globallink, TPT_STATUS_ERROR);
      watchdog(TPT_OBJECT_TYPE_GLOBALLINK, 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
      form_set_error('', check_plain('Error: ' . $e->getMessage()));
    }
  }

  return $count;
}

function transperfect_get_translated_taxonomy_items($xml) {
  if (is_null($xml) || !is_string($xml) || $xml == '') {
    return array();
  }

  $dom = new DomDocument;
  $dom->preserveWhiteSpace = FALSE;
  $dom->loadXML($xml);

  $contents = $dom->getElementsByTagName('content');

  $bid = '';

  foreach ($contents as $content) {
    if (!is_null($content->attributes)) {
      foreach ($content->attributes as $attr_name => $attr_node) {
        if ($attr_name == 'bid') {
          $bid = $attr_node->value;
        }
      }
    }
  }

  if ($bid == '') {
    return array();
  }

  $taxonomy_arr = array();
  $taxonomy_arr['bid'] = $bid;

  $taxonomies = $dom->getElementsByTagName('taxonomy');

  foreach ($taxonomies as $taxonomy) {
    if (!is_null($taxonomy->attributes)) {
      $b_arr = array();

      foreach ($taxonomy->attributes as $attr_name => $attr_node) {
        $b_arr[$attr_name] = $attr_node->value;
      }

      $b_arr['translation'] = $taxonomy->nodeValue;
      $taxonomy_arr[$b_arr['name']] = $b_arr;
    }
  }

  return $taxonomy_arr;
}

function transperfect_update_taxonomy_status(&$globallink, $status = TPT_STATUS_PENDING_TRANSLATIONS) {
  db_update('transperfect_core_taxonomy')
  ->fields(array('status' => $status, 'timestamp' => REQUEST_TIME))
  ->condition('submission', $globallink->submissionName, '=')
  ->condition('document_ticket', $globallink->documentTicket, '=')
  ->condition('source', $globallink->sourceLocale, '=')
  ->condition('target', $globallink->targetLocale, '=')
  ->execute();
}

function transperfect_get_active_taxonomy_submission_rows($object_id) {
  $query = db_select('transperfect_core_taxonomy', 'tc');
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

  foreach ($arr as $bid => $bid_arr) {
    $sub_arr = array();

    foreach ($bid_arr as $r) {
      if (array_key_exists($r->submission, $sub_arr)) {
        array_push($sub_arr[$r->submission], $r->target);
      }
      else {
        $sub_arr[$r->submission] = array($r->target);
      }
    }

    if (count($sub_arr) > 0) {
      $final_arr[$bid] = $sub_arr;
    }
  }

  if (count($final_arr) > 0) {
    return $final_arr;
  }

  return FALSE;
}

function transperfect_get_sent_taxonomy_rows_by_bid($bid) {
  $result = db_select('transperfect_core_taxonomy', 'tc')
  ->fields('tc')
  ->condition('object_id', $bid, '=')
  ->condition('status', TPT_STATUS_SENT_FOR_TRANSLATIONS, '=')
  ->execute();

  $rows = array();

  foreach ($result as $row) {
    $rows[] = $row;
  }

  return $rows;
}

function transperfect_get_taxonomy_row_id_from_submission($submission_ticket, $document_ticket, $target_locale) {
  $result = db_select('transperfect_core_taxonomy', 'tc')
  ->condition('submission_ticket', $submission_ticket, '=')
  ->condition('document_ticket', $document_ticket, '=')
  ->condition('target', $target_locale, '=')
  ->fields('tc')
  ->execute();

  foreach ($result as $row) {
    return $row->rid;
  }
}

function transperfect_get_arr_taxonomy_items($xml) {
  if (is_null($xml) || !is_string($xml) || $xml == '') {
    return array();
  }

  $dom = new DomDocument;
  $dom->preserveWhiteSpace = FALSE;
  $dom->loadXML($xml);

  $contents = $dom->getElementsByTagName('content');
  $bid = '';

  foreach ($contents as $content) {
    if (!is_null($content->attributes)) {
      foreach ($content->attributes as $attr_name => $attr_node) {
        if ($attr_name == 'bid') {
          $bid = $attr_node->value;
        }
      }
    }
  }

  if ($bid == '') {
    return array();
  }

  $block_arr = array();
  $block_arr['bid'] = $bid;
  $blocks = $dom->getElementsByTagName('taxonomy');

  foreach ($blocks as $block) {
    if (!is_null($block->attributes)) {
      $b_arr = array();

      foreach ($block->attributes as $attr_name => $attr_node) {
        $b_arr[$attr_name] = $attr_node->value;
      }

      $block_arr[$b_arr['name']] = $block->nodeValue;
    }
  }

  return $block_arr;
}

function transperfect_get_vocabulary_list() {
  $v_arr = array('[any]' => 'Any');
  $arr = array();

  $vocabs = taxonomy_vocabulary_load_multiple(FALSE);

  foreach ($vocabs as $vocab) {
    if ($vocab->i18n_mode == 1) {
      $arr[$vocab->vid] = $vocab->name;
    }
  }

  asort($arr);

  return $v_arr + $arr;
}

function transperfect_get_distinct_active_taxonomy_submission_names() {
  $query = db_select('transperfect_core_taxonomy', 'tc');
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

function transperfect_update_taxonomy_submission_status($submission_ticket, $status = TPT_STATUS_CANCELLED) {
  db_update('transperfect_core_taxonomy')
  ->fields(array(
    'status' => $status,
    'timestamp' => REQUEST_TIME,
  ))
  ->condition('submission_ticket', $submission_ticket, '=')
  ->execute();
}

function transperfect_get_taxonomy_submission_status() {
  $query = db_select('transperfect_core_taxonomy', 'tc');
  $query->fields('tc', array('submission_ticket'));
  $query->distinct();
  $query->condition('status', TPT_STATUS_SENT_FOR_TRANSLATIONS, '=');
  $results = $query->execute();

  foreach ($results as $row) {
    if ($row->submission_ticket) {
      try {
        $pd4 = transperfect_get_project_director_details();

        $doc_status = transperfect_get_status($pd4, $row->submission_ticket);

        if (!$doc_status || $doc_status == PD_STATUS_CANCELLED) {
          transperfect_update_taxonomy_submission_status($row->submission_ticket);
        }
      }
      catch (SoapFault $se) {
        transperfect_update_taxonomy_submission_status($row->submission_ticket);
      }
      catch (Exception $ex) {
        transperfect_update_taxonomy_submission_status($row->submission_ticket);
      }
    }
  }
}

function transperfect_check_taxonomy_status($rids_arr) {
  $status = TRUE;

  $query = db_select('transperfect_core_taxonomy', 'tc')
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

function transperfect_clear_cancelled_taxonomy_documents() {
  $count = 0;

  $query = db_select('transperfect_core_taxonomy', 'tc')
  ->fields('tc', array('submission_ticket'))
  ->distinct()
  ->condition('status', TPT_STATUS_CANCELLED, '=');

  $results = $query->execute();

  foreach ($results as $item) {
    transperfect_update_taxonomy_submission_status($item->submission_ticket, TPT_STATUS_PENDING_TRANSLATIONS);

    $count++;
  }

  return $count;
}
