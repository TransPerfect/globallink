<?php

require_once TPT_ROOT . '/' . ('transperfect_core_constants.inc');
require_once TPT_ROOT . '/' . ('transperfect_settings.inc');
require_once TPT_ROOT . '/' . ('transperfect_common.inc');
require_once TPT_ROOT . '/' . ('transperfect_fieldable_panels/transperfect_fieldable_panels.inc');
require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_send_translations.inc');

function transperfect_dashboard_fieldable_panels($type) {
  $_SESSION['transperfect_selected_type'] = $type;
  $array = array();
  $array[] = drupal_get_form('transperfect_dashboard_fieldable_panels_filter_form');
  $array[] = drupal_get_form('transperfect_dashboard_fieldable_panels_pager_form');
  $array[] = drupal_get_form('transperfect_dashboard_fieldable_panels_form');

  return $array;
}

function transperfect_dashboard_fieldable_panels_filter_form() {
  $form = array();
  $locales = get_mapped_drupal_locales(FALSE);
  if (!empty($locales)) {
    $filter = FALSE;

    foreach ($locales as $key => $lang) {
      $lang_filter[$key] = $lang;
    }

    $default_language = language_default();
    $default = $default_language->language;
    if (!empty($_SESSION['transperfect_selected_fieldable_panels_language'])) {
      $default = $_SESSION['transperfect_selected_fieldable_panels_language'];
      $filter = TRUE;
    }

    $form['select_fieldable_panels_type'] = array(
        '#type' => 'fieldset',
        '#title' => t('FILTER FIELDABLE PANELS PANES'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
    );

    $form['select_fieldable_panels_type']['filters'] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('container-inline')),
    );

    $form['select_fieldable_panels_type']['filters']['language'] = array(
        '#type' => 'select', '#title' => 'Language' . ':&nbsp;&nbsp;',
        '#options' => $lang_filter,
        '#default_value' => $default,
        '#prefix' => '<div class="container-inline"><table><tr><td style="text-align: left;width: 15%;vertical-align: middle;border: none;">',
        '#suffix' => '</tr></table></div>',
        '#field_suffix' => '</td>',
        '#field_prefix' => '</td><td>',
    );

    $disable_submit = $filter ? TRUE : FALSE;
    $form['select_fieldable_panels_type']['filters']['filter'] = array(
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
      $form['filter-search']['filter-markup'][] = array('#markup' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'><b>" . "Language" . "</b>:&nbsp;</td><td style='text-align: left;width: 85%;vertical-align: middle;border: none;'>" . $lang_filter[$default] . "</td></tr>");
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
  }
  else {
    $form['locale_mapping']['no_mapping'] = array('#type' => 'markup', '#markup' => t('<br/><b><i>No GlobalLink locale mapping found.</i></b>'));
  }
  return $form;
}

function transperfect_dashboard_fieldable_panels_filter_form_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($op == TPT_BTN_DASHBOARD_FILTER) {
    $_SESSION['transperfect_selected_fieldable_panels_language'] = $form_state['values']['language'];
  }
  elseif ($op == TPT_BTN_DASHBOARD_RESET) {
    $_SESSION['transperfect_selected_fieldable_panels_language'] = '';
  }
}

function transperfect_dashboard_fieldable_panels_pager_form() {
  $module_path = drupal_get_path('module', 'transperfect');
  drupal_add_css($module_path . '/css/transperfect.css');
  $form = array();
  $locales = get_mapped_drupal_locales(FALSE);
  if (!empty($locales)) {
    $form['page_counter']['markup'] = array(
        '#type' => 'container',
        '#attributes' => array('class' => array('tpt-center')),
    );
    $form['page_counter']['markup']['p_count'] = array(
        '#type' => 'textfield',
        '#size' => 5,
        '#default_value' => isset($_SESSION['transperfect_fieldable_panels_page_count']) ? $_SESSION['transperfect_fieldable_panels_page_count'][0] : TPT_PAGER_LIMIT,
        '#field_prefix' => t('Show') . '&nbsp;&nbsp;',
        '#field_suffix' => '&nbsp;&nbsp;' . t('records') . '&nbsp;&nbsp;&nbsp;&nbsp;',
        '#prefix' => '<div class="container-inline">',
    );
    $form['page_counter']['markup']['action'] = array(
        '#type' => 'submit',
        '#value' => TPT_BTN_DASHBOARD_GO,
        '#suffix' => '</div>',
        '#limit_validation_errors' => array(),
        '#submit' => array('transperfect_dashboard_fieldable_panels_pager_form_submit'),
        '#access' => TRUE,
    );
  }
  return $form;
}

function transperfect_dashboard_fieldable_panels_pager_form_validate($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($op == TPT_BTN_DASHBOARD_GO) {
    $page_count = $form_state['input']['p_count'];
    if (!is_numeric($page_count) || $page_count < 1) {
      form_set_error('', t('Invalid Page Count.'));
    }
  }
}

function transperfect_dashboard_fieldable_panels_pager_form_submit($form, &$form_state) {
  $op = $form_state['values']['op'];
  if ($op == TPT_BTN_DASHBOARD_GO) {
    $page_count = $form_state['input']['p_count'];
    if (!is_numeric($page_count) || $page_count < 1) {
      form_set_error('', t('Invalid Page Count.'));
    }
    else {
      $_SESSION['transperfect_fieldable_panels_page_count'] = array($page_count);
      return;
    }
  }
}

function transperfect_dashboard_fieldable_panels_form() {
  $form = array();
  $module_path = drupal_get_path('module', 'transperfect');
  $languages = get_mapped_drupal_locales(FALSE);
  if (isset($languages) && count($languages) > 1) {
    $target_lang_arr = get_mapped_tpt_locales_with_drupal_desc(FALSE);
    $source_lang_arr = get_mapped_tpt_locales_with_drupal_desc(FALSE);

    $default_language = language_default();
    $default = $default_language->language;
    if (!empty($_SESSION['transperfect_selected_fieldable_panels_language'])) {
      $default = $_SESSION['transperfect_selected_fieldable_panels_language'];
    }

    unset($target_lang_arr[get_tpt_locale_code($default)]);

    $t_count = 0;
    foreach ($target_lang_arr as $key => $value) {
      $t_count++;
      if ($t_count % 2 === 0) {
        $target_lang_arr[$key] = '&nbsp;&nbsp;' . $value . '<BR/>';
      }
      else {
        $target_lang_arr[$key] = '&nbsp;&nbsp;' . $value . '&nbsp;&nbsp;&nbsp;&nbsp;';
      }
    }

    $page_count = TPT_PAGER_LIMIT;
    if (isset($_SESSION['transperfect_fieldable_panels_page_count'])) {
      $page_count = $_SESSION['transperfect_fieldable_panels_page_count'][0];
    }

    $header = array(
        'title' => array('field' => 'fpp.title', 'data' => t('Fieldable Panels Pane Title')),
        'category' => array('field' => 'fpp.category', 'data' => t('Category')),
        'lang' => t('Language'),
        'active' => t('In Active Submission'),
    );
    //SELECT * FROM `drupal7-fresh`.fieldable_panels_panes;
    $query = db_select('fieldable_panels_panes', 'fpp')
            ->fields('fpp')
            ->condition('language', $default, '=')
            ->extend('PagerDefault')     //Pager Extender
            ->limit($page_count)              //10 results per page
            ->extend('TableSort');        //Sorting Extender

    $sort_order = isset($_GET['order']) ? $_GET['order'] : '';
    if ($sort_order == '') {
      $query->orderBy('fpid', 'DESC');
    }
    else {
      $query->orderByHeader($header);
    }

    $results = $query->execute();

    $count = 0;
    $rows = array();
    foreach ($results as $item) {
      $count++;
      $active = '';
      $active_subs = get_active_fieldable_panels_submission_rows($item->fpid);
      if ($active_subs && array_key_exists($item->fpid, $active_subs)) {
        $sub_arr = $active_subs[$item->fpid];
        $sub_name = "";
        $tgt_arr = array();
        foreach ($sub_arr as $sub_name => $tgt_arr) {
          $l_title = format_display_string($sub_name);
          $l_href = transperfect_get_root_menu('transperfect-translations/activeSubmissions/fieldable_panels');
          $l_options = array('query' => array('submission' => urlencode($sub_name)));
          $active .= l($l_title, $l_href, $l_options) . "&nbsp;&nbsp;- &nbsp;&nbsp;";
          $t_count = 0;
          foreach ($tgt_arr as $tgt) {
            $t_count++;
            $tgt_name = $source_lang_arr[$tgt];
            if ($t_count == count($tgt_arr)) {
              $active .= $tgt_name . " <BR/> ";
            }
            else {
              $active .= $tgt_name . ", &nbsp;&nbsp;";
            }
          }
        }
      }

      $rows[$item->fpid] = array(
          'title' => $item->title,
          'category' => $item->category,
          'lang' => $languages[$item->language],
          'active' => $active,
      );
    }

    $form['table'] = array(
         '#type' => 'tableselect',
         '#header' => $header,
         '#options' => $rows,
         '#empty' => t('No items available'),
    );

    if ($count > 0) {
      $form['pager'] = array('#markup' => theme('pager'));
    }

    if (user_access(TPT_ROLE_DO_TRANSALTIONS) || user_access(TPT_ROLE_MANAGE_TRANSLATIONS)) {
      if ($count > 0) {
        $proj_arr = variable_get('transperfect_pd_projects', array());
        if (count($proj_arr) > 0) {
          $projects = variable_get('transperfect_pd_projectid', "");
          if ($projects != '') {
            $arr = explode(',', $projects);
            foreach ($arr as $value) {
              if (isset($proj_arr[$value])) {
                $proj_arr[$value] = $proj_arr[$value];
              }
              else {
                $proj_arr[$value] = $value;
              }
            }
          }
        }
        $p_key = '';
        if (count($proj_arr) == 1) {
          $p_key = key($proj_arr);
        }
        $proj_arr[''] = ' -- Select a Project -- ';
        $r_proj_arr = array_reverse($proj_arr);
        $form['send_submission'] = array(
            '#type' => 'fieldset',
            '#title' => t('Create Submission'),
            '#collapsible' => TRUE,
            '#collapsed' => TRUE,
        );
        $form['send_submission']['submission_name'] = array(
            '#type' => 'textfield', '#title' => 'Submission Name',
            '#size' => 40, '#required' => TRUE,
            '#default_value' => variable_get('transperfect_pd_submissionprefix', 'DRU_') . REQUEST_TIME,
            '#prefix' => '<div class="container-inline"><table><tr><td style="text-align: left;width: 20%;vertical-align: middle;border: none;">',
            '#suffix' => '</tr>',
            '#field_suffix' => '</td>',
            '#field_prefix' => '<BR/><div class="description">' . t('Name for this new Submission.') . '</div></td><td>',
        );
        $default_date = date('m/d/Y', REQUEST_TIME + (7 * 24 * 60 * 60));
        drupal_add_library('system', 'ui.datepicker');
        drupal_add_js('jQuery(document).ready(function(){
          jQuery( ".pickadate1" ).datepicker({
          dateFormat: "mm/dd/yy",
          defaultDate: "' . $default_date . '",
          minDate: 0,
          autoSize: true,
          showOn: "button",
          buttonImage: "' . $GLOBALS['base_url'] . '/' . $module_path . '/css/icon-calendar.png",
          buttonImageOnly: true,
          buttonText: "Click to select date"
          });});', 'inline');
        $form['send_submission']['submission_due_date'] = array(
            '#type' => 'textfield',
            '#title' => 'Submission Due Date',
            '#required' => TRUE,
            '#attributes' => array('class' => array('pickadate1'),
                'style' => array('padding:0px;')),
            '#default_value' => $default_date,
            '#prefix' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'>",
            '#suffix' => '</tr>',
            '#field_suffix' => '</td>',
            '#field_prefix' => '<BR/><div class="description">' . t('Expected Completion Date for this Submission.') . '</div></td><td>',
        );
        $form['send_submission']['submission_pd_project'] = array(
            '#type' => 'select', '#title' => t('Project Name'), '#required' => TRUE,
            '#options' => $r_proj_arr, '#default_value' => $p_key,
            '#prefix' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'>",
            '#suffix' => '</tr>',
            '#field_suffix' => '</td>',
            '#field_prefix' => '<BR/><div class="description">' . t('GlobalLink Project Name for this Submission') . '</div></td><td>',
        );
        $form['send_submission']['submission_source_locale'] = array(
            '#type' => 'select', '#title' => t('Source Language'), '#required' => TRUE,
            '#options' => $source_lang_arr, '#default_value' => get_tpt_locale_code($default),
            '#disabled' => TRUE,
            '#prefix' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'>",
            '#suffix' => '</tr>',
            '#field_suffix' => '</td>',
            '#field_prefix' => '<BR/><div class="description">' . t('Source language for this Submission.') . '</div></td><td>',
        );
        $form['send_submission']['submission_target_locale'] = array('#type' => 'checkboxes',
            '#options' => $target_lang_arr,
            '#title' => t('Target Language(s)'),
            '#required' => TRUE,
            '#prefix' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'>",
            '#suffix' => '</tr>',
            '#field_suffix' => '</td>',
            '#field_prefix' => '<BR/><div class="description">' . t('Target language(s) for this Submission.') . '</div></td><td>',
        );
        global $user;
        $form['send_submission']['submission_submitter'] = array(
            '#type' => 'textfield', '#title' => 'Submitter',
            '#size' => 40, '#required' => TRUE,
            '#default_value' => check_plain($user->name),
            '#prefix' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'>",
            '#suffix' => '</tr>',
            '#field_suffix' => '</td>',
            '#field_prefix' => '<BR/><div class="description">' . t('Username for this new Submission.') . '</div></td><td>',
        );
        $form['send_submission']['submission_instructions'] = array('#type' => 'textarea',
            '#title' => t('Instructions'),
            '#attributes' => array('style' => 'width: 275px;resize: none;'),
            '#rows' => 3,
            '#resizable' => FALSE,
            '#prefix' => "<tr><td style='text-align: left;width: 15%;vertical-align: middle;border: none;'>",
            '#suffix' => '</tr></table></div>',
            '#field_suffix' => '</div></td>',
            '#field_prefix' => '<BR/><div class="description">' . t('Other instructions for this Submission.') . '</div></td><td>',
        );
        $form['send_submission']['submit_pd_send'] = array('#type' => 'submit', '#value' => TPT_BTN_DASHBOARD_SEND);
      }
    }
  }
  return $form;
}

function transperfect_dashboard_fieldable_panels_form_validate($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  $pd4 = get_project_director_details();
  validate_project_director_details($pd4);
  $source_locale = $form_state['values']['submission_source_locale'];
  $target_locale_arr = $form_state['values']['submission_target_locale'];

  if ($op == TPT_BTN_DASHBOARD_SEND) {
    $fpids = array_filter($form_state['values']['table']);
    if (count($fpids) == 0) {
      $fpids = array_filter($form_state['values']['table']);
      form_set_error('', t('No items selected.'));
    }
    if (!empty($form_state['values']['submission_due_date'])) {
      $due_date = (convert_date_to_timestamp($form_state['values']['submission_due_date'])) / 1000;
      if ($due_date < REQUEST_TIME) {
        form_set_error('', t('The specified date is in past.'));
      }
    }
    if (count($fpids) > 0) {
      if (isset($source_locale) && isset($target_locale_arr)) {
        foreach ($target_locale_arr as $key => $target) {
          if ($target == FALSE) {
            unset($target_locale_arr[$key]);
          }
        }
        $message = FALSE;
        if (count($target_locale_arr) > 0) {
          foreach ($fpids as $fpid) {
            $rows = get_sent_tpt_fieldable_panels_rows_by_fpid($fpid);
            foreach ($rows as $row) {
              if ($row->source == $source_locale && array_search($row->target, $target_locale_arr)) {
                $message = TRUE;
              }
            }
          }
          if ($message) {
            drupal_set_message(t('Ignored record(s) that has already been sent out for translation.'), 'warning');
          }
        }
      }
    }
  }
}

function transperfect_dashboard_fieldable_panels_form_submit($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  $submission_details = array();
  if ($op == TPT_BTN_DASHBOARD_SEND) {
    try {
      $pd4 = get_project_director_details();
      $fpids = array_filter($form_state['values']['table']);
      $submission_name = $form_state['values']['submission_name'];
      $source_locale = $form_state['values']['submission_source_locale'];
      $target_locale_arr = $form_state['values']['submission_target_locale'];
      $submission_details['instructions'] = $form_state['values']['submission_instructions'];
      $submission_details['submitter'] = $form_state['values']['submission_submitter'];
      foreach ($target_locale_arr as $key => $target) {
        if ($target == FALSE) {
          unset($target_locale_arr[$key]);
        }
      }
      $due_date = convert_date_to_timestamp($form_state['values']['submission_due_date']);
      $project_code = $form_state['values']['submission_pd_project'];
      $updated_globalLink_arr = send_fieldable_panels_pane_for_translations($fpids, $pd4, $submission_name, $due_date, $project_code, $source_locale, $target_locale_arr, $submission_details);
      if (count($updated_globalLink_arr) > 0) {
        update_fieldable_panels_ticket_id($updated_globalLink_arr, $project_code);
        drupal_set_message(t('Content has been successfully sent for translations.'));
      }
    }
    catch (Exception $ex) {
      form_set_error('', check_plain('Error: ' . $ex->getMessage()));
    }
  }
}