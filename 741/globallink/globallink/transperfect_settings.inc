<?php

require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_common.inc');

function transperfect_settings_page() {
  $array = array();

  $array[] = drupal_get_form('transperfect_pd_settings');
  $array[] = drupal_get_form('transperfect_adaptor_settings');

  return $array;
}

function transperfect_pd_settings() {
  $form = array();

  $form['transperfect_pd_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('GlobalLink Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['transperfect_pd_settings']['transperfect_pd_url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#default_value' => variable_get('transperfect_pd_url', ''),
    '#required' => TRUE,
    '#size' => 50,
  );

  $form['transperfect_pd_settings']['transperfect_pd_username'] = array(
    '#type' => 'textfield',
    '#title' => t('User Id'),
    '#default_value' => variable_get('transperfect_pd_username', ''),
    '#required' => TRUE,
    '#size' => 20,
  );

  $form['transperfect_pd_settings']['transperfect_pd_password'] = array(
    '#type' => 'password',
    '#title' => t('Password'),
    '#required' => TRUE,
    '#size' => 20,
  );

  $form['transperfect_pd_settings']['transperfect_pd_projectid'] = array(
    '#type' => 'textfield',
    '#title' => t('Project Short Code(s)'),
    '#description' => t('Enter comma separated codes for multiple projects.'),
    '#default_value' => variable_get('transperfect_pd_projectid', ''),
    '#required' => TRUE,
    '#size' => 50,
  );

  $form['transperfect_pd_settings']['transperfect_pd_submissionprefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Submission Name Prefix'),
    '#default_value' => variable_get('transperfect_pd_submissionprefix', 'DRU_'),
    '#required' => FALSE,
    '#size' => 20,
  );

  $form['transperfect_pd_settings']['transperfect_pd_classifier'] = array(
    '#type' => 'textfield',
    '#title' => t('Classifier'),
    '#default_value' => variable_get('transperfect_pd_classifier', 'XML'),
    '#required' => TRUE,
    '#size' => 10,
  );

  $form['transperfect_pd_settings']['transperfect_pd_max_target'] = array(
    '#type' => 'textfield',
    '#title' => t('Max Target Count'),
    '#default_value' => variable_get('transperfect_pd_max_target', '99999'),
    '#required' => TRUE,
    '#size' => 10,
  );

  $form['transperfect_pd_settings']['submit_pd_test'] = array(
    '#type' => 'submit',
    '#value' => TPT_BTN_SETTINGS_SAVE_TEST_PD,
    '#suffix' => '</div>',
  );

  return $form;
}

function transperfect_pd_settings_validate(&$form, &$form_state) {
  $project_short_code_char_arr = str_split(str_replace(' ', '', $form_state['values']['transperfect_pd_projectid']));

  foreach ($project_short_code_char_arr as $val) {
    if (!preg_match('/[a-zA-Z0-9\,_]/', $val)) {
      form_set_error('', t('Only enter comma seperated GlobalLink Project Short Code.'));

      return FALSE;
    }

    if ($val === strtolower($val) && !is_numeric($val) && $val != ',' && $val != '_' && $val != '') {
      form_set_error('', t('GlobalLink Project Short Code is in lowercase.'));

      return FALSE;
    }
  }

  $project_short_code_arr = explode(',', str_replace(' ', '', $form_state['values']['transperfect_pd_projectid']));

  foreach ($project_short_code_arr as $val) {
    if (count(array_keys($project_short_code_arr, $val)) > 1) {
      form_set_error('', t('GlobalLink Project Short Code has duplicates.'));

      return FALSE;
    }
  }

  $value = $form_state['values']['transperfect_pd_max_target'];

  if (!is_numeric($value)) {
    form_set_error('transperfect_pd_max_target', t('Max Target Count is not a number.'));

    return FALSE;
  }
  elseif ($value < 1) {
    form_set_error('transperfect_pd_max_target', t('Max Target Count should be greater than 1.'));

    return FALSE;
  }

  $prefix_len = strlen($form_state['values']['transperfect_pd_submissionprefix']);

  if ($prefix_len > 118) {
    form_set_error('transperfect_pd_max_target', t('Submission Name cannot be longer than 118 characters. Currently ' . $prefix_len . ' characters long.'));

    return FALSE;
  }

  if (isset($form_state['values']['transperfect_pd_password']) && $form_state['values']['transperfect_pd_password'] != '') {
    try {
      $pd_obj = new ProjectDirector();

      $url = strrev($form_state['values']['transperfect_pd_url']);

      if (ord($url) == 47) {
        $url = substr($url, 1);
      }

      $r_url = strrev($url);

      $pd_obj->url = $r_url;
      $pd_obj->username = $form_state['values']['transperfect_pd_username'];
      $pd_obj->password = $form_state['values']['transperfect_pd_password'];
      $pd_obj->projectShortCode = $form_state['values']['transperfect_pd_projectid'];
      $pd_obj->submissionPrefix = $form_state['values']['transperfect_pd_submissionprefix'];
      $pd_obj->classifier = $form_state['values']['transperfect_pd_classifier'];
      $pd_obj->maxTargetCount = $form_state['values']['transperfect_pd_max_target'];

      $success = transperfect_test_pd_connectivity($pd_obj);

      if (!is_bool($success)) {
        form_set_error('', check_plain($success));
      }
    }
    catch (SoapFault $se) {
      watchdog(TPT_OBJECT_TYPE_GLOBALLINK, 'SOAP Exception - %function - Code[%faultcode], Message[%faultstring]', array('%function' => __FUNCTION__, '%faultcode' => $se->faultcode, '%faultstring' => $se->faultstring), WATCHDOG_ERROR);

      form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
    }
    catch (Exception $e) {
      watchdog(TPT_OBJECT_TYPE_GLOBALLINK, 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);

      form_set_error('', check_plain('Error: ' . $e->getMessage()));
    }
  }
}

function transperfect_pd_settings_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';

  switch ($op) {
    case TPT_BTN_SETTINGS_PD:
      foreach ($form_state['values'] as $key => $value) {
        if (is_array($value) && isset($form_state['values']['array_filter'])) {
          $value = array_keys(array_filter($value));
        }

        variable_set($key, $value);
      }

      transperfect_save_project_names();
      drupal_set_message(t('The configuration options have been saved.'));

      break;
    case TPT_BTN_SETTINGS_SAVE_TEST_PD:
      try {
        foreach ($form_state['values'] as $key => $value) {
          if (is_array($value) && isset($form_state['values']['array_filter'])) {
            $value = array_keys(array_filter($value));
          }

          variable_set($key, $value);
        }

        transperfect_save_project_names();
        drupal_set_message(t('Settings Saved and Connection Test Successful.'));
      }
      catch (SoapFault $se) {
        watchdog(TPT_OBJECT_TYPE_GLOBALLINK, 'SOAP Exception - %function - Code[%faultcode], Message[%faultstring]', array('%function' => __FUNCTION__, '%faultcode' => $se->faultcode, '%faultstring' => $se->faultstring), WATCHDOG_ERROR);

        form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
      }
      catch (Exception $e) {
        watchdog(TPT_OBJECT_TYPE_GLOBALLINK, 'Exception - %function - File[%file], Line[%line], Code[%code], Message[%message]', array('%function' => __FUNCTION__, '%file' => $e->getFile(), '%line' => $e->getLine(), '%code' => $e->getCode(), '%message' => $e->getMessage()), WATCHDOG_ERROR);

        form_set_error('', check_plain('Error: ' . $e->getMessage()));
      }

      break;
  }
}

function transperfect_adaptor_settings() {
  $form = array();

  $form['transperfect_adaptor_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Adaptor Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['transperfect_adaptor_settings']['transperfect_pager_limit'] = array(
    '#type' => 'textfield',
    '#title' => t('Dashboard Pager Limit'),
    '#default_value' => variable_get('transperfect_pager_limit', 10),
    '#required' => TRUE,
    '#size' => 20,
  );

  $form['transperfect_adaptor_settings']['transperfect_enable_preview'] = array(
    '#type' => 'radios',
    '#title' => t('Enable Preview For Receive Translations'),
    '#default_value' => variable_get('transperfect_enable_preview', 1),
    '#options' => array(t('No'), t('Yes')),
  );

  $form['transperfect_adaptor_settings']['transperfect_implementation_type'] = array(
    '#type' => 'radios',
    '#title' => t('Node/Field Translation Filter Implementation'),
    '#default_value' => variable_get('transperfect_implementation_type', 0),
    '#description' => t('Changing this value will remove and reload the contents on the Dashboard based on the selected value. Please make sure that there are no active submissions.'),
    '#options' => array(t('Standard'), t('Using Hook (Refer documentation and hook_sample.php)')),
  );

  $form['transperfect_adaptor_settings']['transperfect_publish_node'] = array(
    '#type' => 'radios',
    '#title' => t('Publish Translated Content'),
    '#default_value' => variable_get('transperfect_publish_node', 0),
    '#options' => array(t('No'), t('Yes'), t('Use Source Content Setting')),
  );

  $form['transperfect_adaptor_settings']['transperfect_cron_type'] = array(
    '#type' => 'radios',
    '#title' => t('Automatic Update Status'),
    '#default_value' => variable_get('transperfect_cron_type', 0),
    '#options' => array(t('Disabled'), t('Drupal Cron'), t('External Cron (transperfect_cron.php)')),
  );

  $form['transperfect_adaptor_settings']['transperfect_proxy_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Proxy URL'),
    '#default_value' => variable_get('transperfect_proxy_url', ''),
    '#required' => FALSE,
    '#size' => 50,
  );

  $form['transperfect_adaptor_settings']['submit_general_save'] = array(
    '#type' => 'submit',
    '#value' => TPT_BTN_SETTINGS_GENERAL,
  );

  return $form;
}

function transperfect_adaptor_settings_validate($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';

  if ($op == TPT_BTN_SETTINGS_GENERAL) {
    $value = $form_state['values']['transperfect_pager_limit'];

    if (!is_numeric($value)) {
      form_set_error('transperfect_pager_limit', t('Dahsboard Pager Limit is not a number.'));
    }
    elseif ($value < 1) {
      form_set_error('transperfect_pager_limit', t('Dahsboard Pager Limit should be greater than 1.'));
    }

    $value = $form_state['values']['transperfect_implementation_type'];

    if ($value != variable_get('transperfect_implementation_type')) {
      if (transperfect_pending_submissions_exists()) {
        form_set_error('', t('Active submission exists. Please make sure there are no active submissions before changing this value.'));
      }
    }
  }
}

function transperfect_adaptor_settings_submit($form, &$form_state) {
  $old_value = variable_get('transperfect_implementation_type');
  $old_preview = variable_get('transperfect_enable_preview', 1);

  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';

  if ($op == TPT_BTN_SETTINGS_GENERAL) {
    foreach ($form_state['values'] as $key => $value) {
      if (is_array($value) && isset($form_state['values']['array_filter'])) {
        $value = array_keys(array_filter($value));
      }

      variable_set($key, $value);
    }

    $value = $form_state['values']['transperfect_implementation_type'];

    if ($value != $old_value) {
      db_truncate('transperfect_core')->execute();
    }

    $pvalue = $form_state['values']['transperfect_enable_preview'];

    if ($pvalue != $old_preview) {
      $_SESSION['transperfect_globalLink_arr'] = array();

      unset($_SESSION['transperfect_globalLink_arr_last_refreshed']);
    }
  }

  drupal_set_message(t('The configuration options have been saved.'));
}

function transperfect_get_project_director_details() {
  $pd4 = new ProjectDirector();

  $url = strrev(variable_get('transperfect_pd_url', ''));

  if (ord($url) == 47) {
    $url = substr($url, 1);
  }

  $r_url = strrev($url);

  $pd4->url = $r_url;
  $pd4->username = variable_get('transperfect_pd_username', '');
  $pd4->password = variable_get('transperfect_pd_password', '');
  $pd4->projectShortCode = variable_get('transperfect_pd_projectid', '');
  $pd4->submissionPrefix = variable_get('transperfect_pd_submissionprefix', '');
  $pd4->classifier = variable_get('transperfect_pd_classifier', 'XML');
  $pd4->maxTargetCount = variable_get('transperfect_pd_max_target', '99999');

  return $pd4;
}

function transperfect_save_project_names() {
  $pd4 = transperfect_get_project_director_details();

  $projects_arr = transperfect_get_user_pd_projects($pd4);
  $project_short_code = $pd4->projectShortCode;

  $var_arr = array();

  if ($project_short_code != '') {
    $proj_code_arr = explode(',', $project_short_code);

    foreach ($proj_code_arr as $proj_code) {
      if (isset($projects_arr[$proj_code])) {
        $var_arr[$proj_code] = $projects_arr[$proj_code];
      }
    }
  }

  variable_set('transperfect_pd_projects', $var_arr);
}

function transperfect_validate_project_director_details($pd4) {
  if (empty($pd4->url)) {
    form_set_error('', t('GlobalLink URL is undefined.'));

    return FALSE;
  }
  elseif (empty($pd4->username)) {
    form_set_error('', t('GlobalLink User Id is undefined.'));

    return FALSE;
  }
  elseif (empty($pd4->password)) {
    form_set_error('', t('GlobalLink Password is undefined.'));

    return FALSE;
  }
  elseif (empty($pd4->projectShortCode)) {
    form_set_error('', t('GlobalLink Project Code is undefined.'));

    return FALSE;
  }
  elseif (empty($pd4->classifier)) {
    form_set_error('', t('GlobalLink Classifier is undefined.'));

    return FALSE;
  }
  elseif (empty($pd4->maxTargetCount)) {
    form_set_error('', t('GlobalLink Max Target Count is undefined.'));

    return FALSE;
  }
  elseif (!is_numeric($pd4->maxTargetCount)) {
    form_set_error('', t('GlobalLink Max Target Count is not a number.'));
    
    return FALSE;
  }

  return TRUE;
}

function transperfect_view_log($from_date) {
  $from_date = transperfect_convert_date_to_timestamp($from_date);
}
