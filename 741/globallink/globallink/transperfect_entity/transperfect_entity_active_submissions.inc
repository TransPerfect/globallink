<?php

module_load_include('inc', 'transperfect', 'transperfect');

function transperfect_entity_active_submissions($type) {
  $_SESSION['transpefect_entity_active_type'] = $type;

  $array = array();
  $array[] = drupal_get_form('transperfect_entity_active_select_form');
  $array[] = drupal_get_form('transperfect_entity_active_pager_form');
  $array[] = drupal_get_form('transperfect_entity_active_form');

  return $array;
}

function transperfect_entity_active_select_form() {
  $form = array();

  get_entity_submission_status();

  $redirect_submission = isset($_GET['submission']) ? urldecode($_GET['submission']) : '';
  $selected_value = '';
  $options = transperfect_get_distinct_entity_active_submission_names();

  if (empty($redirect_submission) && !empty($_SESSION['transperfect_entity_selected_submission'])) {
    $selected_value = $_SESSION['transperfect_entity_selected_submission'];
  }
  elseif (!empty($redirect_submission)) {
    $selected_value = array_search($redirect_submission, $options);
  }

  $form['#attributes']['class'][] = 'transperfect-entity-active-select-form';

  $form['select_submission'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('container-inline')),
  );

  $form['select_submission']['submission'] = array(
    '#type' => 'select',
    '#title' => t('Active Submission Name:'),
    '#options' => $options,
    '#default_value' => $selected_value,
    '#attributes' => array(
      'class' => array(
        'transperfect-entity-select-active-submission',
      ),
    ),
    '#field_suffix' => '&nbsp;&nbsp;&nbsp;&nbsp;',
  );

  $form['select_submission']['go'] = array(
    '#type' => 'submit',
    '#value' => t('Go'),
    '#attributes' => array('style' => 'display: none;'),
  );

  if ($selected_value != '') {
    $form['select_submission']['submit_cancel_submission'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel Submission'),
    );
  }
  else {
    $form['select_submission']['br_markup'] = array(
      '#type' => 'markup',
      '#markup' => '<BR/><BR/>',
    );
  }

  return $form;
}

function transperfect_entity_active_select_form_submit($form, &$form_state) {
  if (isset($_GET['submission'])) {
    unset($_GET['submission']);
  }

  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  $pd4 = transperfect_get_project_director_details();

  switch ($op) {
    case t('Cancel Submission'):
      try {
        sleep(5);
        $selected_submission = $form_state['values']['submission'];
        $_SESSION['transperfect_entity_selected_submission'] = '';

        transperfect_cancel_entity_submission($selected_submission, $pd4);

        $_SESSION['transperfect_entity_globalLink_arr'] = array();

        drupal_set_message(t('Submission has been cancelled successfully.'));
      }
      catch (SoapFault $se) {
        watchdog('GlobalLink', 'SOAP Exception - %function - Code[%faultcode], Message[%faultstring]', array('%function' => __FUNCTION__, '%faultcode' => $se->faultcode, '%faultstring' => $se->faultstring), WATCHDOG_ERROR);
        form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
      }
      catch (Exception $e) {
        watchdog('GlobalLink', 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
        form_set_error('', check_plain('Error: ' . $e->getMessage()));
      }

      break;
    case t('Go'):
      $_SESSION['transperfect_entity_selected_submission'] = $form_state['values']['submission'];

      break;
  }
}

function transperfect_entity_active_pager_form() {
  $module_path = drupal_get_path('module', 'transperfect');
  drupal_add_css($module_path . '/css/transperfect.css');

  $form = array();

  $form['page_counter']['markup'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('tpt-center')),
  );

  $form['page_counter']['markup']['p_count'] = array(
    '#type' => 'textfield',
    '#size' => 5,
    '#default_value' => isset($_SESSION['transperfect_entity_active_page_count']) ? $_SESSION['transperfect_entity_active_page_count'][0] : TPT_PAGER_LIMIT,
    '#field_prefix' => t('Show') . '&nbsp;&nbsp;',
    '#field_suffix' => '&nbsp;&nbsp;' . t('records') . '&nbsp;&nbsp;&nbsp;&nbsp;',
    '#prefix' => '<div class="container-inline">',
  );

  $form['page_counter']['markup']['action'] = array(
    '#type' => 'submit',
    '#value' => t('Go'),
    '#suffix' => '</div>',
    '#limit_validation_errors' => array(),
    '#submit' => array('node_active_pager_form_submit'),
    '#access' => TRUE,
  );

  return $form;
}

function transperfect_entity_active_pager_form_submit($form, &$form_state) {
  $op = $form_state['values']['op'];

  if ($op != t('Go')) {
    return;
  }

  $page_count = $form_state['input']['p_count'];

  if (!is_numeric($page_count) || $page_count < 1) {
    form_set_error('', t('Invalid Page Count.'));
  }
  else {
    $_SESSION['transperfect_entity_active_page_count'] = array($page_count);

    return;
  }
}

function transperfect_entity_active_form() {
  $form = array();

  $redirect_submission = isset($_GET['submission']) ? urldecode($_GET['submission']) : '';
  $selected_value = '';

  if (empty($redirect_submission) && !empty($_SESSION['transperfect_entity_selected_submission'])) {
    $selected_value = $_SESSION['transperfect_entity_selected_submission'];
  }
  elseif (!empty($redirect_submission)) {
    $options = transperfect_get_distinct_entity_active_submission_names();
    $selected_value = array_search($redirect_submission, $options);
  }

  if (!empty($_SESSION['transpefect_entity_active_type'])) {
    $content_types = node_type_get_names();
    $projects = transperfect_get_pd_projects();

    $page_count = TPT_PAGER_LIMIT;

    if (isset($_SESSION['transperfect_entity_active_page_count'])) {
      $page_count = $_SESSION['transperfect_entity_active_page_count'][0];
    }

    $header = array(
      'submission' => array(
        'field' => 'submission',
        'data' => t('Submission Name'),
      ),
      'type' => array(
        'field' => 'type',
        'data' => t('Content Type'),
      ),
      'title' => array(
        'field' => 'title',
        'data' => t('Title'),
      ),
      'project_code' => array(
        'field' => 'project_code',
        'data' => t('Project'),
      ),
      'source_name' => array(
        'field' => 'source_name',
        'data' => t('Source Language'),
      ),
      'target_name' => array(
        'field' => 'target_name',
        'data' => t('Target Language'),
      ),
      'status' => array(
        'field' => 'status',
        'data' => t('Status'),
      ),
      'timestamp' => array(
        'field' => 'timestamp',
        'data' => t('Last Updated'),
      ),
    );

    $query = db_select('transperfect_core_entity', 'tc')->extend('PagerDefault') //Pager Extender
    ->limit($page_count) // 10 results per page
    ->extend('TableSort') // Sorting Extender
    ->orderByHeader($header); // Field to sort on is picked from $header
    $query->condition('status', array('Sent for Translations', 'Error', 'Cancelled'), 'IN');

    if ($selected_value != '') {
      $query->condition('submission_ticket', $selected_value, '=');
    }

    $query->join('transperfect_locale', 'tl1', 'tc.source = tl1.locale_code');
    $query->join('transperfect_locale', 'tl2', 'tc.target = tl2.locale_code');
    $query->fields('tc');
    $query->addField('tl1', 'drupal_locale_desc', 'source_name');
    $query->addField('tl2', 'drupal_locale_desc', 'target_name');

    $results = $query->execute();

    $count = 0;
    $rows = array();

    foreach ($results as $item) {
      $count++;

      $rows[$item->rid] = array(
        'rid' => $item->rid,
        'nid' => $item->nid,
        'vid' => $item->vid,
        'submission' => $item->submission,
        'title' => l(transperfect_format_display_string($item->title), 'node/' . $item->nid),
        'project_code' => isset($projects[$item->project_code]) ? $projects[$item->project_code] : '',
        'type' => isset($content_types[$item->type]) ? $content_types[$item->type] : '',
        'status' => $item->status,
        'timestamp' => format_date($item->timestamp, 'custom', 'Y-m-d H:i:s'),
        'source_name' => $item->source_name,
        'target_name' => $item->target_name,
      );
    }

    $form['table'] = array(
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $rows,
      '#empty' => t('No items available'),
    );

    $form['pager'] = array('#markup' => theme('pager'));

    if ($count > 0) {
      if (user_access(TPT_ROLE_MANAGE_TRANSLATIONS)) {
        $form['submit_cancel_document'] = array(
          '#type' => 'submit',
          '#value' => t('Cancel Selected Documents'),
        );

        $form['submit_clear_cancelled_documents'] = array(
          '#type' => 'submit',
          '#value' => t('Clear Cancelled Documents'),
        );
      }
    }
  }

  return $form;
}

function transperfect_entity_active_form_validate($form, &$form_state) {
  $pd4 = transperfect_get_project_director_details();
  transperfect_validate_project_director_details($pd4);

  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';

  if ($op == t('Cancel Selected Documents')) {
    $rids = array_filter($form_state['values']['table']);

    if (!isset($rids) || empty($rids)) {
      form_set_error('', t('No items selected.'));
    }
  }
}

function transperfect_entity_active_form_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  $pd4 = transperfect_get_project_director_details();

  switch ($op) {
    case t('Cancel Selected Documents'):
      try {
        sleep(5);

        $rids_arr = array_filter($form_state['values']['table']);

        $rids = transperfect_check_entity_status($rids_arr);

        if (count($rids) == 0) {
          drupal_set_message(t('No document(s) cancelled.'), 'warning', NULL);

          return;
        }

        $_SESSION['transperfect_entity_selected_submission'] = '';

        transperfect_cancel_select_entity_records($rids, $pd4);

        $_SESSION['transperfect_entity_globalLink_arr'] = array();
        drupal_set_message(t('Selected document(s) has been cancelled successfully.'));
      }
      catch (SoapFault $se) {
        $_SESSION['transperfect_entity_selected_submission'] = '';
        watchdog('GlobalLink', 'SOAP Exception - %function - Code[%faultcode], Message[%faultstring]', array('%function' => __FUNCTION__, '%faultcode' => $se->faultcode, '%faultstring' => $se->faultstring), WATCHDOG_ERROR);
        form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
      }
      catch (Exception $e) {
        $_SESSION['transperfect_entity_selected_submission'] = '';
        watchdog('GlobalLink', 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
        form_set_error('', check_plain('Error: ' . $e->getMessage()));
      }

      break;
    case t('Clear Cancelled Documents'):
      try {
        $count = transperfect_clear_cancelled_entity_documents();
        $_SESSION['transperfect_entity_selected_submission'] = '';
        $_SESSION['transperfect_entity_globalLink_arr'] = array();

        if ($count > 0) {
          drupal_set_message(t('Cancelled document(s) has been cleared successfully.'));
        }
        else {
          drupal_set_message(t('No document(s) to clear.'), 'warning', NULL);
        }
      }
      catch (Exception $e) {
        watchdog('GlobalLink', 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);
        form_set_error('', check_plain('Error: ' . $e->getMessage()));
      }

      break;
  }
}
