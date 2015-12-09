<?php

function transperfect_dashboard_receive_page($type) {
  $_SESSION['transpefect_dashboard_receive_type'] = $type;

  $array = array();

  $array[] = drupal_get_form('transperfect_node_receive_filter_form');
  $array[] = drupal_get_form('transperfect_node_receive_pager_form');
  $array[] = drupal_get_form('transperfect_dashboard_receive');

  return $array;
}

function transperfect_node_receive_filter_form() {
  module_load_include('inc', 'transperfect', 'transperfect');
  module_load_include('inc', 'transperfect', 'transperfect_node');

  $form = array();

  $submission_options = transperfect_get_distinct_active_submission_names();
  unset($submission_options['']);

  $submission_options['[any]'] = 'Any';
  $default_submission = '[any]';

  if (!empty($_SESSION['transperfect_node_receive_filter_submission'])) {
    if (!in_array($_SESSION['transperfect_node_receive_filter_submission'], $submission_options)) {
      $_SESSION['transperfect_node_receive_filter_submission'] = '';
      $_SESSION['transperfect_globalLink_arr'] = array();

      unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);
    }
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
    'Translation Completed' => 'Translation Completed',
    'Error' => 'Error',
    'Source Deleted' => 'Source Deleted',
  );

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

  $form['node_receive_filter']['filters'][] = array('#markup' => '<table>');

  $form['node_receive_filter']['filters']['language'] = array(
    '#type' => 'select', '#title' => t('Target Language'),
    '#options' => $languages,
    '#default_value' => $default,
    '#field_suffix' => '&nbsp;&nbsp;&nbsp;&nbsp;',
    '#prefix' => '<tr><td style="text-align: left;width: 15%;vertical-align: middle;border: none;">',
    '#suffix' => '</tr>',
    '#field_suffix' => '</td>',
    '#field_prefix' => '</td><td>',
  );

  $form['node_receive_filter']['filters']['status'] = array(
    '#type' => 'select', '#title' => t('Status'),
    '#options' => $status_arr,
    '#default_value' => $default_status,
    '#field_suffix' => '&nbsp;&nbsp;&nbsp;&nbsp;',
    '#prefix' => '<tr><td style="text-align: left;width: 15%;vertical-align: middle;border: none;">',
    '#suffix' => '</tr>',
    '#field_suffix' => '</td>',
    '#field_prefix' => '</td><td>',
  );

  $form['node_receive_filter']['filters']['submission'] = array(
    '#type' => 'select', '#title' => t('Submission'),
    '#options' => $submission_options,
    '#default_value' => $default_submission,
    '#field_suffix' => '&nbsp;&nbsp;&nbsp;&nbsp;',
    '#prefix' => '<tr><td style="text-align: left;width: 15%;vertical-align: middle;border: none;">',
    '#suffix' => '</tr>',
    '#field_suffix' => '</td>',
    '#field_prefix' => '</td><td>',
  );

  $form['node_receive_filter']['filters'][] = array('#markup' => '</table>');

  $disable_submit = $filter ? TRUE : FALSE;

  $form['node_receive_filter']['filters']['filter'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
    '#disabled' => $disable_submit,
  );

  if ($filter) {
    $form['filter-search'] = array(
      '#type' => 'fieldset',
      '#title' => t('FILTER CRITERIA'),
    );

    $form['filter-search']['filter-markup'] = array(
      '#type' => 'container',
    );

    $form['filter-search']['filter-markup'][] = array('#markup' => '<table>');

    $form['filter-search']['filter-markup'][] = array(
      '#prefix' => '<tr><td style="text-align: left;width: 15%;vertical-align: middle;border: none;"><b>',
      '#markup' => t('Target Language'),
      '#suffix' => '</b>:&nbsp;</td><td style="text-align: left;width: 85%;vertical-align: middle;border: none;">' . $languages[$default] . '</td></tr>',
    );

    $form['filter-search']['filter-markup'][] = array(
      '#prefix' => '<tr><td style="text-align: left;width: 15%;vertical-align: middle;border: none;"><b>',
      '#markup' => t('Status'),
      '#suffix' => '</b>:&nbsp;</td><td style="text-align: left;width: 85%;vertical-align: middle;border: none;">' . $status_arr[$default_status] . '</td></tr>',
    );

    $form['filter-search']['filter-markup'][] = array(
      '#prefix' => '<tr><td style="text-align: left;width: 15%;vertical-align: middle;border: none;"><b>',
      '#markup' => t('Submission'),
      '#suffix' => '</b>:&nbsp;</td><td style="text-align: left;width: 85%;vertical-align: middle;border: none;">' . $submission_options[$default_submission] . '</td></tr>',
    );

    $form['filter-search']['filter-markup'][] = array('#markup' => '</table>');

    $form['filter-search']['status']['actions'] = array(
      '#type' => 'actions',
      '#attributes' => array(
        'class' => array('container-inline'),
      ),
    );

    $form['filter-search']['status']['actions']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Reset'),
    );
  }

  return $form;
}

function transperfect_node_receive_filter_form_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';

  switch ($op) {
    case t('Filter'):
      $_SESSION['transperfect_node_receive_filter_language'] = $form_state['values']['language'];
      $_SESSION['transperfect_node_receive_filter_status'] = $form_state['values']['status'];
      $_SESSION['transperfect_node_receive_filter_submission'] = $form_state['values']['submission'];
      $_SESSION['transperfect_globalLink_arr'] = array();

      unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);

      break;
    case t('Reset'):
      $_SESSION['transperfect_node_receive_filter_language'] = '';
      $_SESSION['transperfect_node_receive_filter_status'] = '';
      $_SESSION['transperfect_node_receive_filter_submission'] = '';
      $_SESSION['transperfect_globalLink_arr'] = array();

      unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);

      break;
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
    '#value' => t('Go'),
    '#suffix' => '</div>',
    '#limit_validation_errors' => array(),
    '#submit' => array('transperfect_node_receive_pager_form_submit'),
    '#access' => TRUE,
  );

  return $form;
}

function transperfect_node_receive_pager_form_submit($form, &$form_state) {
  $op = $form_state['values']['op'];

  if ($op != t('Go')) {
    return;
  }

  $page_count = $form_state['input']['p_count'];

  if (!is_numeric($page_count) || $page_count < 1) {
    form_set_error('', t('Invalid Page Count.'));
  }
  else {
    $_SESSION['transperfect_receive_page_count'] = array($page_count);

    return;
  }
}

function transperfect_dashboard_receive() {
  module_load_include('inc', 'transperfect', 'transperfect_settings');
  module_load_include('inc', 'transperfect', 'gl_ws/gl_ws_receive_translations');
  
  $form = array();

  if (!empty($_SESSION['transpefect_dashboard_receive_type'])) {
    $type = $_SESSION['transpefect_dashboard_receive_type'];

    $globallink_arr = array();

    if (empty($_SESSION['transperfect_globalLink_arr'])) {
      try {
        $pd4 = transperfect_get_project_director_details();

        if (transperfect_validate_project_director_details($pd4)) {
          $_SESSION['transperfect_globalLink_arr_last_refreshed'] = format_date(REQUEST_TIME, 'custom', 'D M j, Y G:i:s');

          $globallink_arr = transperfect_get_ready_translations_details_from_pd($pd4, $type);

          if (sizeof($globallink_arr) > 0) {
            $_SESSION['transperfect_globalLink_arr'] = serialize($globallink_arr);
          }
        }
      }
      catch (SoapFault $se) {
        watchdog('GlobalLink', 'SOAP Exception - %function - Code[%faultcode], Message[%faultstring]', array('%function' => __FUNCTION__, '%faultcode' => $se->faultcode, '%faultstring' => $se->faultstring), WATCHDOG_ERROR);
        form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
      }
      catch (Exception $e) {
        watchdog('GlobalLink', 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
        form_set_error('', check_plain('Error: ' . $e->getMessage()));
      }
    }
    else {
      $globallink_arr = unserialize($_SESSION['transperfect_globalLink_arr']);
    }

    $header = array(
      'submissionName' => t('Submission'),
      'sourceLocale' => t('Source Language'),
      'targetLocale' => t('Target Language'),
      'title' => t('Title'),
      'status' => t('Status'),
    );

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
        if ($globallink->type != 'node') {
          continue;
        }

        $source_xml = '';

        $status = 'Translation Completed';

        $title = transperfect_get_node_title($globallink);

        if (!$title) {
          $title = $globallink->title;
          $status = 'Source Deleted';
          $source_xml = 'Source Deleted';
          $globallink->status = 'Source Deleted';
          $globallink->sourceDeleted = TRUE;
        }

        if ($globallink->status == 'Error') {
          $status = 'Error';
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
          $rows[$key]['preview'] = $source_xml != 'Source Deleted' ? l(t('Preview Translations'), 'javascript:void(0)', array(
            'fragment' => '',
            'external' => TRUE,
            'attributes' => array(
              'title' => 'Click to Preview',
              'id' => 'tpt_content_link_' . $key,
            ),
          )) : '';
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

          $form['tpt_content_div_' . $lcount] = array(
            '#type' => 'markup',
            '#markup' => '',
            '#prefix' => '<div id="tpt_content_div_' . $lcount . '" style="display: none;">',
            '#suffix' => '</div>',
          );

          $div_arr['tpt_content_link_' . $lcount] = 'tpt_content_div_' . $lcount;
        }
      }

      drupal_add_js(array(
        'transperfect' => array(
          'popup' => $div_arr,
          'previewpath' => base_path() . 'admin/transperfect-translations/preview',
          'rids' => $rid_arr,
        ),
      ), 'setting');
    }

    if ($count > 1) {
      $form['submit_update_selected'] = array(
        '#type' => 'submit',
        '#value' => t('Update Selected'),
      );

      $form['submit_update_all'] = array(
        '#type' => 'submit',
        '#value' => t('Update All'),
      );

      $form['submit_clear_deleted_hidden'] = array(
        '#type' => 'submit',
        '#value' => t('Clear Deleted Records'),
        '#attributes' => array(
          'style' => 'display:none;',
          'disabled' => 'true',
        ),
      );

      $form['submit_clear_deleted'] = array(
        '#type' => 'submit',
        '#value' => t('Clear Deleted Records'),
      );
    }

    $last_refreshed = '';

    if (isset($_SESSION['transperfect_globalLink_arr_last_refreshed'])) {
      $last_refreshed = $_SESSION['transperfect_globalLink_arr_last_refreshed'];
    }

    $form['submit_refresh_hidden'] = array(
      '#type' => 'submit',
      '#value' => t('Refresh Page'),
      '#prefix' => '<BR/>',
      '#attributes' => array(
        'style' => 'display:none;',
        'disabled' => 'true',
      ),
    );

    $form['submit_refresh'] = array(
      '#type' => 'submit',
      '#value' => t('Refresh Page'),
    );

    $form['markup'] = array(
      '#type' => 'markup',
      '#markup' => 'Last refreshed : ' . $last_refreshed,
    );
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

  if ($op == t('Update Selected')) {
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

  switch ($op) {
    case t('Update All'):
      try {
        $globallink_arr = unserialize($_SESSION['transperfect_globalLink_arr']);
        $count = transperfect_get_translated_content($pd4, $globallink_arr);

        $error = FALSE;
        $delete_records_selected = FALSE;

        foreach ($globallink_arr as $updated) {
          if ($updated->sourceDeleted == TRUE) {
            $delete_records_selected = TRUE;
          }

          if ($updated->status == 'Error') {
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
        watchdog('GlobalLink', 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
        form_set_error('', check_plain('Error: ' . $e->getMessage()));
      }

      $_SESSION['transperfect_globalLink_arr'] = array();
      $_SESSION['transperfect_selected_submission'] = array();

      unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);

      break;
    case t('Clear Deleted Records'):
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
        watchdog('GlobalLink', 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
        form_set_error('', check_plain('Error: ' . $e->getMessage()));
      }

      break;
    case t('Update Selected'):
      try {
        $delete_records_selected = FALSE;
        $globallink_arr = unserialize($_SESSION['transperfect_globalLink_arr']);

        $updated_globallink_arr = array();
        $rids = array_filter($form_state['values']['table']);

        foreach ($rids as $rid) {
          array_push($updated_globallink_arr, $globallink_arr[$rid]);
        }

        $count = transperfect_get_translated_content($pd4, $updated_globallink_arr);

        $error = FALSE;

        foreach ($updated_globallink_arr as $updated) {
          if ($updated->sourceDeleted == TRUE) {
            $delete_records_selected = TRUE;
          }

          if ($updated->status == 'Error') {
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
        watchdog('GlobalLink', 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
        form_set_error('', check_plain('Error: ' . $e->getMessage()));
      }

      $_SESSION['transperfect_globalLink_arr'] = array();
      $_SESSION['transperfect_selected_submission'] = array();

      unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);

      break;
    case t('Refresh Page'):
      sleep(2);

      $pd4 = transperfect_get_project_director_details();

      try {
        $globallink_arr = transperfect_get_ready_translations_details_from_pd($pd4, $type);
      }
      catch (SoapFault $se) {
        watchdog('GlobalLink', 'SOAP Exception - %function - Code[%faultcode], Message[%faultstring]', array('%function' => __FUNCTION__, '%faultcode' => $se->faultcode, '%faultstring' => $se->faultstring), WATCHDOG_ERROR);
        form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
      }
      catch (Exception $e) {
        watchdog('GlobalLink', 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
        form_set_error('', check_plain('Error: ' . $e->getMessage()));
      }

      if (sizeof($globallink_arr) > 0) {
        $_SESSION['transperfect_globalLink_arr'] = serialize($globallink_arr);
        $_SESSION['transperfect_globalLink_arr_last_refreshed'] = format_date(REQUEST_TIME, 'custom', 'D M j, Y G:i:s');
      }
      else {
        $_SESSION['transperfect_globalLink_arr'] = array();

        unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);
      }

      break;
  }
}

/**
 * Automatically receives the translated contents.
 *
 * @param string $cron_type
 *   The type of cron job.
 */
function transperfect_auto_receive($cron_type) {
  watchdog('GlobalLink', 'Cron Started by %cron_type', array('%cron_type' => $cron_type), WATCHDOG_INFO);

  try {
    $pd4 = transperfect_get_project_director_details();
    $all_target_arr = transperfect_get_all_ready_translations_details_from_pd($pd4);

    $count = 0;

    foreach ($all_target_arr as $globallink) {
      $type = $globallink->type;
      $globallink_arr = array($globallink);

      switch ($type) {
        case 'node':
          $count = transperfect_get_translated_content($pd4, $globallink_arr);

          watchdog('GlobalLink', 'Imported Node', array(), WATCHDOG_INFO);

          if ($globallink->status == 'Error') {
            transperfect_update_status($globallink);
          }

          $_SESSION['transperfect_globalLink_arr'] = array();
          $_SESSION['transperfect_selected_submission'] = array();

          break;
        case 'entity':
          $count = transperfect_entity_get_translated_entity($pd4, $globallink_arr);

          watchdog('GlobalLink', 'Imported Entity', array(), WATCHDOG_INFO);

          if ($globallink->status == 'Error') {
            transperfect_entity_update_status($globallink);
          }

          $_SESSION['transperfect_entity_globalLink_arr'] = array();
          $_SESSION['transperfect_entity_selected_submission'] = array();

          break;
        case 'block':
          $count += transperfect_block_get_translated_blocks($pd4, $globallink_arr);

          watchdog('Block', t('Imported Block'), array(), WATCHDOG_INFO);

          $_SESSION['transperfect_block_globalLink_arr'] = array();
          $_SESSION['transperfect_block_selected_submission'] = array();

          break;
        case 'interface':
          $count += transperfect_get_translated_interfaces($pd4, $globallink_arr);

          watchdog('GlobalLink', 'Imported Interface', array(), WATCHDOG_INFO);

          $_SESSION['transperfect_interface_globalLink_arr'] = array();
          $_SESSION['transperfect_interface_selected_submission'] = array();

          break;
        case 'menu':
          $count += transperfect_menu_get_translated($pd4, $globallink_arr);

          watchdog('GlobalLink', 'Imported Menu', array(), WATCHDOG_INFO);

          $_SESSION['transperfect_menu_globalLink_arr'] = array();
          $_SESSION['transperfect_menu_selected_submission'] = array();

          break;
        case 'taxonomy':
          $count += transperfect_get_translated_taxonomies($pd4, $globallink_arr);

          watchdog('GlobalLink', 'Imported Taxonomy', array(), WATCHDOG_INFO);

          $_SESSION['transperfect_taxonomy_globalLink_arr'] = array();
          $_SESSION['transperfect_taxonomy_selected_submission'] = array();

          break;
        case 'webform':
          $count += transperfect_get_translated_webforms($pd4, $globallink_arr);

          watchdog('GlobalLink', 'Imported Webform', array(), WATCHDOG_INFO);

          $_SESSION['transperfect_webform_globalLink_arr'] = array();
          $_SESSION['transperfect_webform_selected_submission'] = array();

          break;
        case 'fieldable_panels_panes':
          $count += transperfect_get_translated_fieldable_panels($pd4, $globallink_arr);

          watchdog('GlobalLink', 'Imported Fieldable Panels Panes', array(), WATCHDOG_INFO);

          $_SESSION['transperfect_fieldable_panels_panes_globalLink_arr'] = array();
          $_SESSION['transperfect_fieldable_panels_panes_selected_submission'] = array();

          break;
      }
    }
  }
  catch (SoapFault $se) {
    watchdog('GlobalLink', 'SOAP Exception - %function - Code[%faultcode], Message[%faultstring]', array('%function' => __FUNCTION__, '%faultcode' => $se->faultcode, '%faultstring' => $se->faultstring), WATCHDOG_ERROR);
  }
  catch (Exception $e) {
    watchdog('GlobalLink', 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
  }

  watchdog('GlobalLink', 'Cron Completed - %count Records Updated.', array('%count' => $count), WATCHDOG_INFO);
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
    $targets = transperfect_get_completed_targets_by_doc($pd4, $globallink->documentTicket);

    if (!is_array($targets)) {
      $targets = array($targets);
    }

    foreach ($targets as $target) {
      if (str_replace('-', '_', $target->targetLanguage->locale) != $globallink->targetLocale) {
        $source_xml = TPT_STATUS_SOURCE_DELETED;
        $globallink->sourceDeleted = TRUE;

        continue;
      }

      if (!is_null($target->ticket) && $target->ticket != '') {
        $globallink->targetTicket = $target->ticket;
        $globallink->targetXML = transperfect_download_target_resource($pd4, $globallink->targetTicket);

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

            if ($source_xml != 'Source Deleted') {
              $source_arr = transperfect_get_translated_array($source_xml);
            }
          }
          else {
            $source_xml = 'Source Deleted';
            $globallink->sourceDeleted = TRUE;
          }
        }
      }

      $name = '';

      $source_xml = transperfect_get_xml($node, $globallink->targetLocale, NULL, NULL, $name, TRUE);

      $source_arr = array();

      if ($source_xml != TPT_STATUS_SOURCE_DELETED) {
        $source_arr = transperfect_get_translated_array($source_xml);
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
    $arr = array('Source Deleted');
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
