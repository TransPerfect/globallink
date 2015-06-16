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
      '#default_value' => variable_get('transperfect_pd_url', ""),
      '#required' => TRUE,
      '#size' => 50);

  $form['transperfect_pd_settings']['transperfect_pd_username'] = array(
      '#type' => 'textfield',
      '#title' => t('User Id'),
      '#default_value' => variable_get('transperfect_pd_username', ""),
      '#required' => TRUE,
      '#size' => 20);

  $form['transperfect_pd_settings']['transperfect_pd_password'] = array(
      '#type' => 'password',
      '#title' => t('Password'),
      '#required' => TRUE,
      '#size' => 20);

  $form['transperfect_pd_settings']['transperfect_pd_projectid'] = array(
      '#type' => 'textfield',
      '#title' => t('Project Short Code(s)'),
      '#description' => t('Enter comma separated codes for multiple projects.'),
      '#default_value' => variable_get('transperfect_pd_projectid', ""),
      '#required' => TRUE,
      '#size' => 50);

  $form['transperfect_pd_settings']['transperfect_pd_submissionprefix'] = array(
      '#type' => 'textfield',
      '#title' => t('Submission Name Prefix'),
      '#default_value' => variable_get('transperfect_pd_submissionprefix', "DRU_"),
      '#required' => FALSE,
      '#size' => 20);

  $form['transperfect_pd_settings']['transperfect_pd_classifier'] = array(
      '#type' => 'textfield',
      '#title' => t('Classifier'),
      '#default_value' => variable_get('transperfect_pd_classifier', "XML"),
      '#required' => TRUE,
      '#size' => 10);

  $form['transperfect_pd_settings']['transperfect_pd_max_target'] = array(
      '#type' => 'textfield',
      '#title' => t('Max Target Count'),
      '#default_value' => variable_get('transperfect_pd_max_target', "99999"),
      '#required' => TRUE,
      '#size' => 10);

//  $form['transperfect_pd_settings']['submit_pd_save'] = array('#type' => 'submit', '#value' => TPT_BTN_SETTINGS_PD, '#prefix' => '<div class="form-item">');
  $form['transperfect_pd_settings']['submit_pd_test'] = array('#type' => 'submit', '#value' => TPT_BTN_SETTINGS_SAVE_TEST_PD, '#suffix' => '</div>');

  return $form;
}

function transperfect_pd_settings_validate(&$form, &$form_state) {
  $project_short_code_char_arr = str_split(str_replace(' ', '', $form_state['values']['transperfect_pd_projectid']));
  foreach ($project_short_code_char_arr as $val) {
    if (!preg_match("/[a-zA-Z0-9\,_]/", $val)) {
      form_set_error('', t('Only enter comma seperated GlobalLink Project Short Code.'));
      return FALSE;
    }
    if ($val === strtolower($val) && !is_numeric($val)
            && $val != ',' && $val != '_' && $val != '') {
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
    form_set_error('transperfect_pd_max_target', t("Submission Name cannot be longer than 118 characters. Currently $prefix_len characters long."));
    return FALSE;
  }

  if (isset($form_state['values']['transperfect_pd_password'])
          && $form_state['values']['transperfect_pd_password'] != '') {
    try {
      $pd4Object = new ProjectDirector();
      $url = strrev($form_state['values']['transperfect_pd_url']);
      if (ord($url) == 47) {
        $url = substr($url, 1);
      }
      $r_url = strrev($url);
      $pd4Object->url = $r_url;
      $pd4Object->username = $form_state['values']['transperfect_pd_username'];
      $pd4Object->password = $form_state['values']['transperfect_pd_password'];
      $pd4Object->projectShortCode = $form_state['values']['transperfect_pd_projectid'];
      $pd4Object->submissionPrefix = $form_state['values']['transperfect_pd_submissionprefix'];
      $pd4Object->classifier = $form_state['values']['transperfect_pd_classifier'];
      $pd4Object->maxTargetCount = $form_state['values']['transperfect_pd_max_target'];

      $success = test_ProjectDirector_connectivity($pd4Object);

      if (!is_bool($success)) {
        form_set_error('', check_plain($success));
      }
    }
    catch (SoapFault $se) {
      gl_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "SOAP Exception in Test Connection - Code[$se->faultcode], Message[$se->faultstring]");
      form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
    }
    catch (Exception $e) {
      gl_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in Test Connection - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
      form_set_error('', check_plain('Error: ' . $e->getMessage()));
    }
  }
}

function transperfect_pd_settings_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($op == TPT_BTN_SETTINGS_PD) {
    foreach ($form_state['values'] as $key => $value) {
      if (is_array($value) && isset($form_state['values']['array_filter'])) {
        $value = array_keys(array_filter($value));
      }
      variable_set($key, $value);
    }
    save_project_names();
    drupal_set_message(t('The configuration options have been saved.'));
  }
  elseif ($op == TPT_BTN_SETTINGS_SAVE_TEST_PD) {
    try {
      foreach ($form_state['values'] as $key => $value) {
        if (is_array($value) && isset($form_state['values']['array_filter'])) {
          $value = array_keys(array_filter($value));
        }
        variable_set($key, $value);
      }

      save_project_names();
      drupal_set_message(t('Settings Saved and Connection Test Successful.'));
    }
    catch (SoapFault $se) {
      gl_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "SOAP Exception in Test Connection - Code[$se->faultcode], Message[$se->faultstring]");
      form_set_error('', check_plain('Web Services Error: ' . $se->faultcode . ' - ' . $se->faultstring));
    }
    catch (Exception $e) {
      gl_log(TPT_OBJECT_TYPE_CONTENT, TPT_LOGGING_SEVERITY_ERROR, "Exception in Test Connection - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
      form_set_error('', check_plain('Error: ' . $e->getMessage()));
    }
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

//  $form['transperfect_adaptor_settings']['transperfect_donot_translate_tag'] = array(
//      '#type' => 'textfield',
//      '#title' => t('Do Not Translate Wrapper Tag'),
//      '#default_value' => variable_get('transperfect_donot_translate_tag', ''),
//      '#size' => 30,
//  );
//
//  $form['transperfect_adaptor_settings']['transperfect_donot_translate_text'] = array(
//      '#type' => 'textarea',
//      '#title' => t('Do Not Translate Text'),
//      '#default_value' => variable_get('transperfect_donot_translate_text', ''),
//      '#description' => t('Enter each non-translatable text in a new line.'),
//  );

  $form['transperfect_adaptor_settings']['transperfect_cron_type'] = array(
      '#type' => 'radios',
      '#title' => t('Automatic Update Status'),
      '#default_value' => variable_get('transperfect_cron_type', 0),
      '#options' => array(t('Disabled'), t('Drupal Cron'), t('External Cron (transperfect_cron.php)')),
  );

  $form['transperfect_adaptor_settings']['transperfect_debug_type'] = array(
      '#type' => 'radios',
      '#title' => t('Logging'),
      '#default_value' => variable_get('transperfect_debug_type', 1),
      '#options' => array(TPT_LOGGING_CONFIG_DISABLED => t('Disabled'), TPT_LOGGING_CONFIG_INFO => t('Info'), TPT_LOGGING_CONFIG_DEBUG => t('Debug')),
  );

  //$form['transperfect_adaptor_settings']['view_log'] = array('#type' => 'markup', '#markup' => l(' View Logs ', 'javascript:void(0)', array('external' => TRUE)) . "</br></br>");
  $form['transperfect_adaptor_settings']['markup'] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('tpt-center')),
  );
  $default_date = date('m/d/Y', REQUEST_TIME - (7 * 24 * 60 * 60));
  drupal_add_library('system', 'ui.datepicker');
  drupal_add_js('jQuery(document).ready(function(){
          jQuery( ".pickadate1" ).datepicker({
          dateFormat: "mm/dd/yy",
          defaultDate: "' . $default_date . '",
          maxDate: 0,
          autoSize: true,
          showOn: "button",
          buttonImage: "' . $GLOBALS['base_url'] . '/' . drupal_get_path('module', 'transperfect') . '/css/icon-calendar.png",
          buttonImageOnly: true,
          buttonText: "Click to select date"
          });});', 'inline');
  $form['transperfect_adaptor_settings']['markup']['log_view_date'] = array(
      '#type' => 'textfield',
      '#required' => TRUE,
      '#attributes' => array('class' => array('pickadate1'),
          'style' => array('text-align: left;vertical-align: middle;')),
      '#default_value' => $default_date,
      '#field_prefix' => t('View log since') . '&nbsp;&nbsp;',
      '#field_suffix' => '&nbsp;&nbsp;&nbsp;&nbsp;',
      '#prefix' => '<div class="container-inline">',
  );
  $form['transperfect_adaptor_settings']['markup']['action'] = array(
      '#type' => 'submit',
      '#value' => t(' Download '),
      '#suffix' => '</div></br>',
      '#limit_validation_errors' => array(),
      '#submit' => array('view_log_submit'),
      '#access' => TRUE,
  );

  $form['transperfect_adaptor_settings']['submit_general_save'] = array('#type' => 'submit', '#value' => TPT_BTN_SETTINGS_GENERAL);

  return $form;
}

function view_log_submit($form, &$form_state) {
  $from_date = $form_state['input']['log_view_date'];
  $from_date = convert_date_to_timestamp($from_date) / 1000;
  $destination = 'temporary://' . 'globallink_log.html';
  $my_file_obj = file_unmanaged_save_data(get_log_data($from_date), $destination, FILE_EXISTS_REPLACE);
  file_transfer($my_file_obj, array('Content-Type' => 'application/htm',
      'Content-Disposition' => 'inline; filename="' . 'globallink_log.html' . '"',
      'Content-Length' => filesize($my_file_obj)));
}

function get_log_data($from_date) {
  $html = "<html><head><title>GlobalLink Log</title><style>.datagrid table { border-collapse: collapse; text-align: left; width: 100%; } .datagrid {font: normal 12px/150% Arial, Helvetica, sans-serif; background: #fff; overflow: hidden; border: 1px solid #006699; -webkit-border-radius: 3px; -moz-border-radius: 3px; border-radius: 3px; }.datagrid table td, .datagrid table th { padding: 3px 10px; }.datagrid table thead th {background:-webkit-gradient( linear, left top, left bottom, color-stop(0.05, #006699), color-stop(1, #00557F) );background:-moz-linear-gradient( center top, #006699 5%, #00557F 100% );filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#006699', endColorstr='#00557F');background-color:#006699; color:#FFFFFF; font-size: 15px; font-weight: bold; border-left: 1px solid #0070A8; } .datagrid table thead th:first-child { border: none; }.datagrid table tbody td { color: #00496B; border-left: 1px solid #E1EEF4;font-size: 12px;font-weight: normal; }.datagrid table tbody .alt td { background: #E1EEF4; color: #00496B; }.datagrid table tbody td:first-child { border-left: none; }.datagrid table tbody tr:last-child td { border-bottom: none; }</style></head><body>";
  $result = db_select('transperfect_log', 'tl')
          ->fields('tl')
          ->condition('timestamp', $from_date, '>=')
          ->execute();

  $html .= "<div class=\"datagrid\"><table>";
  $html .= "<thead><tr><th>Date Time</th><th>Log Severity</th><th>Type</th><th>Message</th></tr></thead><tbody>";

  $i = 0;
  foreach ($result as $row) {
    $mod = $i % 2;
    if ($mod == 0) {
      $html .= "<tr><td style='width:10%;'>" . date('Y-m-d H:i:s', $row->timestamp) . "</td><td style='width:10%;'>$row->severity</td><td style='width:10%;'>$row->type</td><td style='width:70%; text-style:justify;'>$row->message</td></tr>";
    }
    else {
      $html .= "<tr class=\"alt\"><td style='width:10%;'>" . date('Y-m-d H:i:s', $row->timestamp) . "</td><td style='width:10%;'>$row->severity</td><td style='width:10%;'>$row->type</td><td style='width:70%; text-style:justify;'>$row->message</td></tr>";
    }
    $i++;
  }

  $html .= "</tbody></table></div>";
  $html .= "</body></html>";

  return $html;
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
      if (pending_submissions_exists()) {
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

function get_project_director_details() {
  $pd4 = new ProjectDirector();
  $url = strrev(variable_get('transperfect_pd_url', ""));
  if (ord($url) == 47) {
    $url = substr($url, 1);
  }
  $r_url = strrev($url);
  $pd4->url = $r_url;
  $pd4->username = variable_get('transperfect_pd_username', "");
  $pd4->password = variable_get('transperfect_pd_password', "");
  $pd4->projectShortCode = variable_get('transperfect_pd_projectid', "");
  $pd4->submissionPrefix = variable_get('transperfect_pd_submissionprefix', "");
  $pd4->classifier = variable_get('transperfect_pd_classifier', "XML");
  $pd4->maxTargetCount = variable_get('transperfect_pd_max_target', "99999");

  return $pd4;
}

function save_project_names() {
  $pd4 = get_project_director_details();
  $projects_arr = get_user_PD_projects($pd4);
  $projectShortCode = $pd4->projectShortCode;
  $var_arr = array();
  if ($projectShortCode != '') {
    $proj_code_arr = explode(',', $projectShortCode);
    foreach ($proj_code_arr as $proj_code) {
      if (isset($projects_arr[$proj_code])) {
        $var_arr[$proj_code] = $projects_arr[$proj_code];
      }
    }
  }
  variable_set('transperfect_pd_projects', $var_arr);
}

function validate_project_director_details($pd4) {
  if ($pd4->url == "") {
    form_set_error('', t('GlobalLink URL is undefined.'));
    return FALSE;
  }
  elseif ($pd4->username == "") {
    form_set_error('', t('GlobalLink User Id is undefined.'));
    return FALSE;
  }
  elseif ($pd4->password == "") {
    form_set_error('', t('GlobalLink Password is undefined.'));
    return FALSE;
  }
  elseif ($pd4->projectShortCode == "") {
    form_set_error('', t('GlobalLink Project Code is undefined.'));
    return FALSE;
  }
  elseif ($pd4->classifier == "") {
    form_set_error('', t('GlobalLink Classifier is undefined.'));
    return FALSE;
  }
  elseif ($pd4->maxTargetCount == "") {
    form_set_error('', t('GlobalLink Max Target Count is undefined.'));
    return FALSE;
  }
  elseif (!is_numeric($pd4->maxTargetCount)) {
    form_set_error('', t('GlobalLink Max Target Count is not a number.'));
    return FALSE;
  }

  return TRUE;
}

function view_log($from_date) {
  $from_date = convert_date_to_timestamp($from_date);
}