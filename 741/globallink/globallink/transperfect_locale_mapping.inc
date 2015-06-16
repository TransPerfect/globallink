<?php

require_once TPT_ROOT . '/' . ('transperfect_core_constants.inc');
require_once TPT_ROOT . '/' . ('transperfect_common.inc');

function transperfect_locale_page() {
  return drupal_get_form('transperfect_locale');
}

function transperfect_locale() {
  $form = array();
  $languages = language_list('language', TRUE);
  $mapped_languages = get_mapped_locale_codes();

  if (sizeof($mapped_languages) > 0) {
    foreach ($mapped_languages as $mlang) {
      foreach ($languages as $language) {
        if ($language->language == $mlang) {
          unset($languages[$mlang]);
          break;
        }
      }
    }
  }

  $drupal_list = array();
  foreach ($languages as $language) {
    if ($language->enabled == 1) {
      $drupal_list[$language->language . '|' . $language->name] = $language->name;
    }
  }

  $tpt_list = get_tpt_list();

  $header = array(
      'drupal_locale_code' => array('field' => 'tl.drupal_locale_desc', 'data' => t('Drupal Site Locales')),
      'locale_code' => array('field' => 'tl.locale_desc', 'data' => t('GlobalLink Locales')),
  );

  $query = db_select('transperfect_locale', 'tl')
          ->isNotNull('drupal_locale_code')
          ->fields('tl')
          ->extend('PagerDefault')     //Pager Extender
          ->limit(TPT_PAGER_LIMIT)              //10 results per page
          ->extend('TableSort')        //Sorting Extender
          ->orderByHeader($header)//Field to sort on is picked from $header
  ;

  $results = $query->execute();

  $count = 0;
  $rows = array();

  foreach ($results as $item) {
    $count++;
    $rows[$item->locale_code] = array(
        'drupal_locale_code' => $item->drupal_locale_desc,
        'locale_code' => $item->locale_desc,
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
    $form['submit_locale_delete'] = array('#type' => 'submit', '#value' => TPT_BTN_LOCALE_DELETE);
  }

  $form['locale_mapping'] = array(
      '#title' => t('Locale Mapping'),
      '#type' => 'fieldset',
      '#description' => t('Map the Drupal locales with GlobalLink locales'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE
  );


  if (sizeof($drupal_list) > 0) {
    $form['locale_mapping']['drupal_locale_list'] = array('#title' => t('Drupal Locale'), '#type' => 'select', '#options' => $drupal_list);
    $form['locale_mapping']['tpt_locale_list'] = array('#title' => t('GlobalLink Locale'), '#type' => 'select', '#options' => $tpt_list);
    $form['locale_mapping']['submit_locale_add'] = array('#type' => 'submit', '#value' => TPT_BTN_ADD);
  }
  else {
    $form['locale_mapping']['no_mapping'] = array('#type' => 'markup', '#markup' => t('<br/><b><i>Nothing to Map</i></b>'));
  }

  return $form;
}

function transperfect_locale_validate($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($op == TPT_BTN_LOCALE_DELETE) {
    $rids = array_filter($form_state['values']['table']);
    if (count($rids) == 0) {
      form_set_error('', t('No items selected.'));
    }
  }
}

function transperfect_locale_submit($form, &$form_state) {

  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  if ($op == TPT_BTN_ADD) {
    $drupal_seleced_locale = $form_state['values']['drupal_locale_list'];
    $drupal_locale = explode("|", $drupal_seleced_locale);
    $locale_code = $form_state['values']['tpt_locale_list'];
    add_locale_mapping($drupal_locale[0], $drupal_locale[1], $locale_code);
    drupal_set_message(t('Mapping added successfully.'));
  }
  else {
    $locales_arr = array_filter($form_state['values']['table']);
    foreach ($locales_arr as $locale) {
      $locale_name = get_drupal_locale_name($locale);
      if (check_pending_submission($locale)) {
        delete_locale_mapping($locale);
        drupal_set_message(t($locale_name . ' mapping deleted successfully.'));
      }
      else {
        form_set_error('', t($locale_name . ' mapping cannot be deleted because there is a pending submission.'));
      }
    }
  }
}

function check_pending_submission($locale) {
  $query1 = db_select('transperfect_core', 'tc')
          ->fields('tc', array('rid')) ->condition('tc.status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN')
          ->condition(db_or()->condition('tc.source', $locale)->condition('tc.target', $locale));

  $table_arr = array(
      'tce' => 'transperfect_core_entity',
      'tcb' => 'transperfect_core_block',
      'tci' => 'transperfect_core_interface',
      'tcm' => 'transperfect_core_menu',
      'tct' => 'transperfect_core_taxonomy',
      'tcw' => 'transperfect_core_webform'
  );

  foreach ($table_arr as $key => $table) {
    if (db_table_exists($table)) {
      $query2 = db_select($table, $key)
            ->fields($key, array('rid'))
            ->condition($key . '.status', array(TPT_STATUS_SENT_FOR_TRANSLATIONS, TPT_STATUS_ERROR), 'IN')
            ->condition(db_or()->condition($key . '.source', $locale)->condition($key . '.target', $locale));

      $query1->union($query2, 'UNION ALL');
    }
  }

  $result = $query1->execute();

  foreach ($result as $row) {
    if (isset($row)) {
      return FALSE;
    }
  }

  return TRUE;
}

function add_locale_mapping($drupal_locale, $drupal_locale_desc, $locale_code) {
  db_update('transperfect_locale')
          ->fields(array('drupal_locale_code' => $drupal_locale, 'drupal_locale_desc' => $drupal_locale_desc))
          ->condition('locale_code', $locale_code, '=')
          ->execute();
}

function delete_locale_mapping($locale_code) {
  db_update('transperfect_locale')
          ->fields(array('drupal_locale_code' => NULL))
          ->condition('locale_code', $locale_code, '=')
          ->execute();
}

function get_tpt_list() {
  $languages = array();
  $result = db_select('transperfect_locale', 'tl')
          ->fields('tl')
          ->isNull('drupal_locale_code')
          ->orderBy('locale_desc', 'ASC')
          ->execute();
  foreach ($result as $row) {
    $languages[$row->locale_code] = $row->locale_desc;
  }

  return $languages;
}

function get_mapped_locale_codes() {
  $languages = array();
  $result = db_select('transperfect_locale', 'tl')
          ->fields('tl')
          ->isNotNull('drupal_locale_code')
          ->orderBy('locale_desc', 'ASC')
          ->execute();
  foreach ($result as $row) {
    $languages[$row->locale_code] = $row->drupal_locale_code;
  }

  return $languages;
}

function get_mapped_tpt_locales($remove_default = TRUE) {
  $languages = array();
  $result = db_select('transperfect_locale', 'tl')
          ->fields('tl')
          ->isNotNull('drupal_locale_code')
          ->orderBy('locale_desc', 'ASC')
          ->execute();
  foreach ($result as $row) {
    if ($remove_default) {
      if (language_default()->language == $row->drupal_locale_code) {
        continue;
      }
    }
    $languages[$row->locale_code] = $row->locale_desc;
  }

  return $languages;
}