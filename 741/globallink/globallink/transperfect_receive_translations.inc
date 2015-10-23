<?php

require_once TPT_ROOT . '/' . ('transperfect_settings.inc');
require_once TPT_ROOT . '/' . ('transperfect_node.inc');
require_once TPT_ROOT . '/' . ('transperfect_entity/transperfect_entity.inc');
require_once TPT_ROOT . '/' . ('transperfect_block/transperfect_block.inc');
require_once TPT_ROOT . '/' . ('transperfect_interface/transperfect_interface.inc');
require_once TPT_ROOT . '/' . ('transperfect_menu/transperfect_menu.inc');
require_once TPT_ROOT . '/' . ('transperfect_taxonomy/transperfect_taxonomy.inc');
require_once TPT_ROOT . '/' . ('transperfect_webform/transperfect_webform.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_receive_translations.inc');

function transperfect_dashboard_receive_page($type) {
  $_SESSION['transpefect_dashboard_receive_type'] = $type;
  $array = array();
  $array[] = drupal_get_form('transperfect_node_receive_filter_form');
  $array[] = drupal_get_form('transperfect_node_receive_pager_form');
  $array[] = drupal_get_form('transperfect_dashboard_receive');
  return $array;
}

function transperfect_node_receive_filter_form() {
  $form = array();

  $submission_options = transperfect_get_distinct_active_submission_names();
  unset($submission_options['']);
  $submission_options['[any]'] = 'Any';
  $default_submission = '[any]';
  if (!empty($_SESSION['transperfect_node_receive_filter_submission']) &&
          !in_array($_SESSION['transperfect_node_receive_filter_submission'], $submission_options)) {
    $_SESSION['transperfect_node_receive_filter_submission'] = '';
    $_SESSION['transperfect_globalLink_arr'] = array();
    unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);
  }

  $filter = FALSE;

  if (!empty($_SESSION['transperfect_node_receive_filter_submission'])) {
    $default_submission = $_SESSION['transperfect_node_receive_filter_submission'];
    $filter = TRUE;
  }

  $n_arr = array('[any]' => t('Any'));
  $t_arr = transperfect_get_mapped_locales_with_drupal_desc(FALSE);
  $languages = $n_arr + $t_arr;

  $status_arr = array('[any]' => t('Any'),
      TPT_STATUS_COMPLETED => TPT_STATUS_COMPLETED,
      TPT_STATUS_ERROR => TPT_STATUS_ERROR,
      TPT_STATUS_SOURCE_DELETED => TPT_STATUS_SOURCE_DELETED);

  $default = '[any]';
  if (!empty($_SESSION['transperfect_node_receive_filter_language'])) {
    $default = $_SESSION['transperfect_node_receive_filter_language'];
    $filter = TRUE;
  }

  $default_status = '[any]';
  if (!empty($_SESSION['transperfect_node_receive_filter_status'])) {
    $default_status = $_SESSION['transperfect_node_receive_filter_status'];
    $filter = TRUE;
  }

  $form['node_receive_filter'] = array(
      '#type' => 'fieldset',
      '#title' => t('FILTER RECORDS'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
  );

  $form['node_receive_filter']['filters'] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('container-inline')),
  );

  $form['node_receive_filter']['filters'][] = array('#markup' => "<table>");

  $form['node_receive_filter']['filters']['language'] = array(
      '#type' => 'select', '#title' => TPT_LABEL_TARGET_LANG . ':&nbsp;&nbsp;',
      '#options' => $languages,
      '#default_value' => $default,
      '#field_suffix' => '&nbsp;&nbsp;&nbsp;&nbsp;',
      '#prefix' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'>",
      '#suffix' => '</tr>',
      '#field_suffix' => '</td>',
      '#field_prefix' => '</td><td>',
  );

  $form['node_receive_filter']['filters']['status'] = array(
      '#type' => 'select', '#title' => TPT_LABEL_STATUS . ':&nbsp;&nbsp;',
      '#options' => $status_arr,
      '#default_value' => $default_status,
      '#field_suffix' => '&nbsp;&nbsp;&nbsp;&nbsp;',
      '#prefix' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'>",
      '#suffix' => '</tr>',
      '#field_suffix' => '</td>',
      '#field_prefix' => '</td><td>',
  );

  $form['node_receive_filter']['filters']['submission'] = array(
      '#type' => 'select', '#title' => TPT_LABEL_SUBMISSION . ':&nbsp;&nbsp;',
      '#options' => $submission_options,
      '#default_value' => $default_submission,
      '#field_suffix' => '&nbsp;&nbsp;&nbsp;&nbsp;',
      '#prefix' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'>",
      '#suffix' => '</tr>',
      '#field_suffix' => '</td>',
      '#field_prefix' => '</td><td>',
  );

  $form['node_receive_filter']['filters'][] = array('#markup' => "</table>");

  $disable_submit = $filter ? TRUE : FALSE;
  $form['node_receive_filter']['filters']['filter'] = array(
      '#type' => 'submit',
      '#value' => TPT_BTN_DASHBOARD_FILTER,
      '#disabled' => $disable_submit
  );

  if ($filter) {

    $form['filter-search'] = array(
        '#type' => 'fieldset',
        '#title' => t('FILTER CRITERIA'),
    );

    $form['filter-search']['filter-markup'] = array(
        '#type' => 'container',
    );
    $form['filter-search']['filter-markup'][] = array('#markup' => "<table>");
    $form['filter-search']['filter-markup'][] = array('#markup' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'><b>" . TPT_LABEL_TARGET_LANG . "</b>:&nbsp;</td><td style='text-align: left;width: 85%;vertical-align: middle;border: none;'>" . $languages[$default] . "</td></tr>");
    $form['filter-search']['filter-markup'][] = array('#markup' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'><b>" . TPT_LABEL_STATUS . "</b>:&nbsp;</td><td style='text-align: left;width: 85%;vertical-align: middle;border: none;'>" . $status_arr[$default_status] . "</td></tr>");
    $form['filter-search']['filter-markup'][] = array('#markup' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'><b>" . TPT_LABEL_SUBMISSION . "</b>:&nbsp;</td><td style='text-align: left;width: 85%;vertical-align: middle;border: none;'>" . $submission_options[$default_submission] . "</td></tr>");
    $form['filter-search']['filter-markup'][] = array('#markup' => "</table>");
    $form['filter-search']['status']['actions'] = array(
        '#type' => 'actions',
        '#attributes' => array('class' => array('container-inline')),
    );

    $form['filter-search']['status']['actions']['reset'] = array(
        '#type' => 'submit',
        '#value' => TPT_BTN_DASHBOARD_RESET,
    );
  }
  return $form;
}

function transperfect_node_receive_filter_form_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($op == TPT_BTN_DASHBOARD_FILTER) {
    $_SESSION['transperfect_node_receive_filter_language'] = $form_state['values']['language'];
    $_SESSION['transperfect_node_receive_filter_status'] = $form_state['values']['status'];
    $_SESSION['transperfect_node_receive_filter_submission'] = $form_state['values']['submission'];
    $_SESSION['transperfect_globalLink_arr'] = array();
    unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);
  }
  elseif ($op == TPT_BTN_DASHBOARD_RESET) {
    $_SESSION['transperfect_node_receive_filter_language'] = '';
    $_SESSION['transperfect_node_receive_filter_status'] = '';
    $_SESSION['transperfect_node_receive_filter_submission'] = '';
    $_SESSION['transperfect_globalLink_arr'] = array();
    unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);
  }
}

function transperfect_node_receive_pager_form() {
  $form = array();
  $form['page_counter']['markup'] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('tpt-center')),
  );
  $form['page_counter']['markup']['p_count'] = array(
      '#type' => 'textfield',
      '#size' => 5,
      '#default_value' => isset($_SESSION['transperfect_receive_page_count']) ? $_SESSION['transperfect_receive_page_count'][0] : TPT_PAGER_LIMIT,
      '#field_prefix' => t('Show') . '&nbsp;&nbsp;',
      '#field_suffix' => '&nbsp;&nbsp;' . t('records') . '&nbsp;&nbsp;&nbsp;&nbsp;',
      '#prefix' => '<div class="container-inline">',
  );
  $form['page_counter']['markup']['action'] = array(
      '#type' => 'submit',
      '#value' => TPT_BTN_DASHBOARD_GO,
      '#suffix' => '</div>',
      '#limit_validation_errors' => array(),
      '#submit' => array('transperfect_node_receive_pager_form_submit'),
      '#access' => TRUE,
  );
  return $form;
}

function transperfect_node_receive_pager_form_submit($form, &$form_state) {
  $op = $form_state['values']['op'];
  if ($op == TPT_BTN_DASHBOARD_GO) {
    $page_count = $form_state['input']['p_count'];
    if (!is_numeric($page_count) || $page_count < 1) {
      form_set_error('', t('Invalid Page Count.'));
    }
    else {
      $_SESSION['transperfect_receive_page_count'] = array($page_count);
      return;
    }
  }
}

function transperfect_dashboard_receive() {
  $form = array();

  if (!empty($_SESSION['transpefect_dashboard_receive_type'])) {
    $type = $_SESSION['transpefect_dashboard_receive_type'];

    $globallink_arr = array();

    if (empty($_SESSION['transperfect_globalLink_arr'])) {
      try {
        $pd4 = transperfect_get_project_director_details();
        if (transperfect_validate_project_director_details($pd4)) {
          $_SESSION['transperfect_globalLink_arr_last_refreshed'] = date("D M j G:i:s T Y", REQUEST_TIME);
          $globallink_arr = get_ready_translations_details_from_pd($pd4, $type);
          if (sizeof($globallink_arr) > 0) {
            $_SESSION['transperfect_globalLink_arr'] = serialize($globallink_arr);
          }
        }
      }
      catch (SoapFault $se) {
        transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "SOAP Exception in Receive Translations - transperfect_dashboard_receive - Code[$se->faultcode], Message[$se->faultstring]");
        form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
      }
      catch (Exception $e) {
        transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in Receive Translations - transperfect_dashboard_receive - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
        form_set_error('', check_plain('Error: ' . $e->getMessage()));
      }
    }
    else {
      $globallink_arr = unserialize($_SESSION['transperfect_globalLink_arr']);
    }

    $header = array('submissionName' => TPT_LABEL_SUBMISSION,
        'sourceLocale' => TPT_LABEL_SOURCE_LANG,
        'targetLocale' => TPT_LABEL_TARGET_LANG,
        'title' => TPT_LABEL_TITLE,
        'status' => TPT_LABEL_STATUS);

    if (variable_get('transperfect_enable_preview', 1)) {
      $header['preview'] = '';
    }

    $rows = array();
    $count = 1;

    $filter_lang = '[any]';
    if (!empty($_SESSION['transperfect_node_receive_filter_language'])) {
      $filter_lang = $_SESSION['transperfect_node_receive_filter_language'];
    }

    if (isset($globallink_arr) && is_array($globallink_arr)) {
      foreach ($globallink_arr as $key => &$globallink) {
        if ($filter_lang != '[any]' && $globallink->targetLocale != $filter_lang) {
          unset($globallink_arr[$key]);
        }
      }
    }

    if (isset($globallink_arr) && is_array($globallink_arr)) {
      $filter_status = '[any]';
      if (!empty($_SESSION['transperfect_node_receive_filter_status'])) {
        $filter_status = $_SESSION['transperfect_node_receive_filter_status'];
      }

      $rid_arr = array();
      foreach ($globallink_arr as $key => &$globallink) {
        if ($globallink->type != "Node") {
          continue;
        }

        $source_xml = '';
        $status = TPT_STATUS_COMPLETED;
        $title = transperfect_get_node_title($globallink);
        if (!$title) {
          $title = $globallink->title;
          $status = TPT_STATUS_SOURCE_DELETED;
          $source_xml = TPT_STATUS_SOURCE_DELETED;
          $globallink->status = TPT_STATUS_SOURCE_DELETED;
          $globallink->sourceDeleted = TRUE;
        }
        if ($globallink->status == TPT_STATUS_ERROR) {
          $status = TPT_STATUS_ERROR;
        }

        if ($filter_status != '[any]' && $status != $filter_status) {
          unset($globallink_arr[$key]);
          continue;
        }

        $filter_submission = '[any]';
        if (!empty($_SESSION['transperfect_node_receive_filter_submission'])) {
          $filter_submission = $_SESSION['transperfect_node_receive_filter_submission'];
        }

        if ($filter_submission != '[any]' && $globallink->submissionName != $filter_submission) {
          unset($globallink_arr[$key]);
          continue;
        }

        $tpt_row_id = transperfect_get_row_id_from_submission($globallink->submissionTicket, $globallink->documentTicket, $globallink->targetLocale);
        if ($tpt_row_id) {
          $rid_arr['tpt_content_div_' . $key] = $tpt_row_id;
        }

        $source_name = transperfect_get_drupal_locale_name($globallink->sourceLocale);
        $target_name = transperfect_get_drupal_locale_name($globallink->targetLocale);

        $rows[$key] = array(
            'submissionName' => $globallink->submissionName,
            'sourceLocale' => $source_name,
            'targetLocale' => $target_name,
            'title' => $title,
            'status' => $status,
        );

        if (variable_get('transperfect_enable_preview', 1)) {
          $rows[$key]['preview'] = $source_xml != TPT_STATUS_SOURCE_DELETED ? l(t('Preview Translations'), 'javascript:void(0)', array('fragment' => '', 'external' => TRUE, 'attributes' => array('title' => 'Click to Preview', 'id' => 'tpt_content_link_' . $key))) : '';
        }

        $count++;
      }

      if (sizeof($globallink_arr) > 0) {
        $_SESSION['transperfect_globalLink_arr'] = serialize($globallink_arr);
      }
    }

    $page_count = isset($_SESSION['transperfect_receive_page_count']) ? $_SESSION['transperfect_receive_page_count'][0] : TPT_PAGER_LIMIT;

    $form['table'] = array(
        '#type' => 'tableselect',
        '#header' => $header,
        '#options' => transperfect_pager_array_splice($rows, $page_count),
        '#empty' => t('No items available'),
    );

    $form['pager'] = array('#markup' => theme('pager'));

    if (variable_get('transperfect_enable_preview', 1)) {
      $lcount = 1;
      $div_arr = array();

      foreach ($rid_arr as $key => $rid) {
        if (isset($rid)) {
          $lcount = str_replace('tpt_content_div_', '', $key);
          $form['tpt_content_div_' . $lcount] = array('#type' => 'markup', '#markup' => '', '#prefix' => '<div id="tpt_content_div_' . $lcount . '" style="display: none;">', '#suffix' => '</div>');
          $div_arr['tpt_content_link_' . $lcount] = 'tpt_content_div_' . $lcount;
        }
      }

      drupal_add_js(array(
          'transperfect' => array(
              'popup' => $div_arr,
              'previewpath' => base_path() . transperfect_get_root_menu('transperfect-translations/preview'),
              'rids' => $rid_arr,
              )), 'setting');
    }

    if ($count > 1) {
      $form['submit_update_selected'] = array('#type' => 'submit', '#value' => TPT_BTN_DASHBOARD_UPDATE_SELECTED);
      $form['submit_update_all'] = array('#type' => 'submit', '#value' => TPT_BTN_DASHBOARD_UPDATE_ALL);
      $form['submit_clear_deleted_hidden'] = array('#type' => 'submit', '#value' => TPT_BTN_DASHBOARD_CLEAR_DELETED_RECORDS, '#attributes' => array('style' => 'display:none;', 'disabled' => 'true'));
      $form['submit_clear_deleted'] = array('#type' => 'submit', '#value' => TPT_BTN_DASHBOARD_CLEAR_DELETED_RECORDS);
    }

    $last_refreshed = "";
    if (isset($_SESSION['transperfect_globalLink_arr_last_refreshed'])) {
      $last_refreshed = $_SESSION['transperfect_globalLink_arr_last_refreshed'];
    }
    $form['submit_refresh_hidden'] = array('#type' => 'submit', '#value' => TPT_BTN_DASHBOARD_REFRESH, '#prefix' => '<BR/>', '#attributes' => array('style' => 'display:none;', 'disabled' => 'true'));
    $form['submit_refresh'] = array('#type' => 'submit', '#value' => TPT_BTN_DASHBOARD_REFRESH);
    $form['markup'] = array(
        '#type' => 'markup',
        '#markup' => 'Last refreshed : ' . $last_refreshed);
  }

  drupal_add_js('jQuery(document).ready(function(){jQuery("#edit-submit-refresh").click(function(){jQuery("#edit-submit-refresh").hide();jQuery("#edit-submit-refresh-hidden").css("display","inline");});});', 'inline');
  drupal_add_js('jQuery(document).ready(function(){jQuery("#edit-submit-clear-deleted").click(function(){jQuery("#edit-submit-clear-deleted").hide();jQuery("#edit-submit-clear-deleted-hidden").css("display","inline");});});', 'inline');
  drupal_add_library('system', 'ui.dialog');
  $module_path = drupal_get_path('module', 'transperfect');
  drupal_add_css($module_path . '/css/transperfect.css');
  drupal_add_js($module_path . '/js/transperfect_receive.js');
  return $form;
}

function transperfect_dashboard_receive_validate($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';

  $pd4 = transperfect_get_project_director_details();
  transperfect_validate_project_director_details($pd4);

  if ($op == TPT_BTN_DASHBOARD_UPDATE_SELECTED) {
    $rids = array_filter($form_state['values']['table']);
    if (count($rids) == 0) {
      form_set_error('', t('No items selected.'));
    }
  }
}

function transperfect_dashboard_receive_submit($form, &$form_state) {
  $type = $_SESSION['transpefect_dashboard_receive_type'];
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  $pd4 = transperfect_get_project_director_details();
  if ($op == TPT_BTN_DASHBOARD_UPDATE_ALL) {
    try {
      $globallink_arr = unserialize($_SESSION['transperfect_globalLink_arr']);
      $count = transperfect_get_translated_content($pd4, $globallink_arr);
      $error = FALSE;
      $delete_records_selected = FALSE;
      foreach ($globallink_arr as $updated) {
        if ($updated->sourceDeleted == TRUE) {
          $delete_records_selected = TRUE;
        }
        if ($updated->status == TPT_STATUS_ERROR) {
          transperfect_update_status($updated);
          $error = TRUE;
        }
      }
      $status = 'status';
      if ($error) {
        drupal_set_message(t('There was some error while updating the records. Please check logs for more details.'), 'warning', NULL);
        $status = 'warning';
      }
      drupal_set_message(check_plain($count . ' record(s) updated successfully.'), $status, NULL);
      if ($delete_records_selected) {
        drupal_set_message(t('Source Deleted record(s) cannot be updated.'), 'warning', NULL);
      }
    }
    catch (Exception $e) {
      transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in Receive Translations - Update All - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
      form_set_error('', check_plain('Error: ' . $e->getMessage()));
    }

    $_SESSION['transperfect_globalLink_arr'] = array();
    $_SESSION['transperfect_selected_submission'] = array();
    unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);
  }
  elseif ($op == TPT_BTN_DASHBOARD_CLEAR_DELETED_RECORDS) {
    try {
      sleep(2);
      $globallink_arr = unserialize($_SESSION['transperfect_globalLink_arr']);
      $success = FALSE;
      $count = 0;
      foreach ($globallink_arr as $globallink) {
        if ($globallink->sourceDeleted == TRUE) {
          $success = transperfect_update_deleted_records($pd4, $globallink);
          $count++;
        }
      }
      if ($count == 0) {
        drupal_set_message(t('No source record(s) deleted.'), 'warning', NULL);
      }
      else {
        $_SESSION['transperfect_globalLink_arr'] = array();
        $_SESSION['transperfect_selected_submission'] = array();
        unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);
      }

      if ($success) {
        drupal_set_message(check_plain($count . ' Source Deleted record(s) cleared successfully.'), 'status', NULL);
      }
    }
    catch (Exception $e) {
      transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in Receive Translations - Update Deleted Records - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
      form_set_error('', check_plain('Error: ' . $e->getMessage()));
    }
  }
  elseif ($op == TPT_BTN_DASHBOARD_UPDATE_SELECTED) {
    try {
      $delete_records_selected = FALSE;
      $globallink_arr = unserialize($_SESSION['transperfect_globalLink_arr']);
      $updated_globalLink_arr = array();
      $rids = array_filter($form_state['values']['table']);
      foreach ($rids as $rid) {
        array_push($updated_globalLink_arr, $globallink_arr[$rid]);
      }
      $count = transperfect_get_translated_content($pd4, $updated_globalLink_arr);
      $error = FALSE;
      foreach ($updated_globalLink_arr as $updated) {
        if ($updated->sourceDeleted == TRUE) {
          $delete_records_selected = TRUE;
        }
        if ($updated->status == TPT_STATUS_ERROR) {
          $error = TRUE;
          transperfect_update_status($updated);
        }
      }
      $status = 'status';
      if ($error) {
        drupal_set_message(t('There was some error while updating the records. Please check logs for more details.'), 'warning', NULL);
        $status = 'warning';
      }
      drupal_set_message(check_plain($count . ' record(s) updated successfully.'), $status, NULL);
      if ($delete_records_selected) {
        drupal_set_message(t('Source Deleted record(s) cannot be updated.'), 'warning', NULL);
      }
    }
    catch (Exception $e) {
      transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in Receive Translations - Update Selected - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
      form_set_error('', check_plain('Error: ' . $e->getMessage()));
    }

    $_SESSION['transperfect_globalLink_arr'] = array();
    $_SESSION['transperfect_selected_submission'] = array();
    unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);
  }
  elseif ($op == TPT_BTN_DASHBOARD_REFRESH) {
    sleep(2);
    $pd4 = transperfect_get_project_director_details();
    try {
      $globallink_arr = getReadyTranslationsDetailsFromPD($pd4, $type);
    }
    catch (SoapFault $se) {
      transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "SOAP Exception in Receive Translations - Refresh - Code[$se->faultcode], Message[$se->faultstring]");
      form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
    }
    catch (Exception $e) {
      transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in Receive Translations - Refresh - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
      form_set_error('', check_plain('Error: ' . $e->getMessage()));
    }

    if (sizeof($globallink_arr) > 0) {
      $_SESSION['transperfect_globalLink_arr'] = serialize($globallink_arr);
      $_SESSION['transperfect_globalLink_arr_last_refreshed'] = date("D M j G:i:s T Y", REQUEST_TIME);
    }
    else {
      $_SESSION['transperfect_globalLink_arr'] = array();
      unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);
    }
  }
}

/**
 * Automatically receives the translated contents.
 *
 * @param string $cron_type
 *   The type of cron job.
 */
function transperfect_auto_receive($cron_type) {
  drupal_set_time_limit(0); //set it so your cron wont time out if it takes a long time to process
  transperfect_gl_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_INFO, "Cron Started by $cron_type");
  
  try {
    $pd4 = transperfect_get_project_director_details();
    $all_target_arr = get_all_ready_translations_details_from_pd($pd4);
    $count = 0;
    foreach ($all_target_arr as $globallink) {
      $type = $globallink->type;
      $globallink_arr = array($globallink);
      switch ($type) {
        case "Node":
          $count = transperfect_get_translated_content($pd4, $globallink_arr);
          transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_DEBUG, "Imported Node");
          if ($globallink->status == TPT_STATUS_ERROR) {
            transperfect_update_status($globallink);
          }
          $_SESSION['transperfect_globalLink_arr'] = array();
          $_SESSION['transperfect_selected_submission'] = array();
          break;
        case "Entity":
          $count = get_translated_entity($pd4, $globallink_arr);
          transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_DEBUG, "Imported Entity");
          if ($globallink->status == TPT_STATUS_ERROR) {
            update_tpt_entity_status($globallink);
          }
          $_SESSION['transperfect_entity_globalLink_arr'] = array();
          $_SESSION['transperfect_entity_selected_submission'] = array();
          break;
        case "Block":
          $count += get_translated_blocks($pd4, $globallink_arr);
          transperfect_log(TPT_OBJECT_TYPE_BLOCK, TPT_LOGGING_SEVERITY_DEBUG, "Imported Block");
          $_SESSION['transperfect_block_globalLink_arr'] = array();
          $_SESSION['transperfect_block_selected_submission'] = array();
          break;
        case "Interface":
          $count += get_translated_interfaces($pd4, $globallink_arr);
          transperfect_log(TPT_OBJECT_TYPE_INTERFACE, TPT_LOGGING_SEVERITY_DEBUG, "Imported Interface");
          $_SESSION['transperfect_interface_globalLink_arr'] = array();
          $_SESSION['transperfect_interface_selected_submission'] = array();
          break;
        case "Menu":
          $count += get_translated_menus($pd4, $globallink_arr);
          transperfect_log(TPT_OBJECT_TYPE_MENU, TPT_LOGGING_SEVERITY_DEBUG, "Imported Menu");
          $_SESSION['transperfect_menu_globalLink_arr'] = array();
          $_SESSION['transperfect_menu_selected_submission'] = array();
          break;
        case "Taxonomy":
          $count += get_translated_taxonomies($pd4, $globallink_arr);
          transperfect_log(TPT_OBJECT_TYPE_TAXONOMY, TPT_LOGGING_SEVERITY_DEBUG, "Imported Taxonomy");
          $_SESSION['transperfect_taxonomy_globalLink_arr'] = array();
          $_SESSION['transperfect_taxonomy_selected_submission'] = array();
          break;
        case "Webform":
          $count += get_translated_webforms($pd4, $globallink_arr);
          transperfect_log(TPT_OBJECT_TYPE_WEBFORM, TPT_LOGGING_SEVERITY_DEBUG, "Imported Webform");
          $_SESSION['transperfect_webform_globalLink_arr'] = array();
          $_SESSION['transperfect_webform_selected_submission'] = array();
          break;
        case "Fieldable_Panels_Panes":
          $count += get_translated_fieldable_panels($pd4, $globallink_arr);
          transperfect_log(TPT_OBJECT_TYPE_FIELDABLE_PANELS, TPT_LOGGING_SEVERITY_DEBUG, "Imported Fieldable Panels Panes");
          $_SESSION['transperfect_fieldable_panels_panes_globalLink_arr'] = array();
          $_SESSION['transperfect_fieldable_panels_panes_selected_submission'] = array();
          break;
      }
    }
  }
  catch (SoapFault $se) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "SOAP Exception in Receive Translations Cron - Code[$se->faultcode], Message[$se->faultstring]");
  }
  catch (Exception $e) {
    transperfect_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in Receive Translations Cron - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
  }

  transperfect_log(TPT_OBJECT_TYPE_GLOBALLINK, TPT_LOGGING_SEVERITY_INFO, "Cron Completed - $count Records Updated.");
}

function transperfect_preview_translated_content() {
  $row_id = $_POST['rid'];
  $arr = transperfect_get_translations_for_row_id($row_id);
  drupal_json_output($arr);
}

function transperfect_get_translations_for_row_id($row_id) {
  $row = transperfect_get_row($row_id);

  $globallink = new GlobalLink();
  $globallink->tptRowId = $row_id;
  $globallink->nid = $row->nid;
  $globallink->vid = $row->vid;
  $globallink->type = $row->type;
  $globallink->title = $row->title;
  $globallink->sourceLocale = $row->source;
  $globallink->targetLocale = $row->target;
  $globallink->documentTicket = $row->document_ticket;
  $globallink->submissionTicket = $row->submission_ticket;
  $globallink->submission = $row->submission;
  $globallink->status = $row->status;

  $source_arr = array();
  $translation = array();

  $error = FALSE;
  try {
    $pd4 = transperfect_get_project_director_details();
    $targets = get_completed_targets_by_doc($pd4, $globallink->documentTicket);

    if (!is_array($targets)) {
      $targets = array($targets);
    }

    foreach ($targets as $target) {
      if (str_replace('-', '_', $target->targetLanguage->locale) == $globallink->targetLocale) {
        if (!is_null($target->ticket) && $target->ticket != '') {
          $globallink->targetTicket = $target->ticket;
          $globallink->targetXML = download_target_resource($pd4, $globallink->targetTicket);
          if (!is_null($globallink->targetXML) && $globallink->targetXML != '') {
            $translation = transperfect_get_translated_array($globallink->targetXML);
            $nid = $translation['nid'];
            $vid = $translation['vid'];
            $node = node_load($nid, $vid);
            if ($node && is_object($node) && !is_null($node)) {
              $drupal_locale_code = transperfect_get_drupal_locale_code($globallink->sourceLocale);
              if ($node->tnid != 0 && $node->language != $drupal_locale_code) {
                $translations = translation_node_get_translations($node->tnid);
                if (isset($translations) && isset($translations[$drupal_locale_code])) {
                  $src_tnode = $translations[$drupal_locale_code];
                  $node = node_load($src_tnode->nid);
                }
              }
              $name = '';
              $source_xml = transperfect_get_xml($node, $globallink->targetLocale, NULL, NULL, $name, TRUE);
              $source_arr = array();
              if ($source_xml != TPT_STATUS_SOURCE_DELETED) {
                $source_arr = transperfect_get_translated_array($source_xml);
              }
            }
            else {
              $source_xml = TPT_STATUS_SOURCE_DELETED;
              $globallink->sourceDeleted = TRUE;
            }
          }
        }
      }
    }
  }
  catch (SoapFault $se) {
    $error = $se->faultstring;
  }
  catch (Exception $ex) {
    $error = $ex->getMessage();
  }

  $arr = array();
  $arr['target_ticket'] = $globallink->targetTicket;
  if ($globallink->sourceDeleted) {
    $arr = array(TPT_STATUS_SOURCE_DELETED);
  }
  elseif ($error) {
    $arr = array('error' => $error);
  }
  else {
    $arr['source'] = $source_arr;
    $arr['target'] = $translation;
  }

  return $arr;
}
