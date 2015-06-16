<?php

/*
 * This file provides an example for the
 * implementation of hooks that other modules can implement.
 * Please read documentation for more details.
 */

/*
 * By implementing this hook, you can programmatically filter out if a particular
 * node is translatable or not by displaying/hiding it on the Send For Translation dashboard.
 * If the node is not translatable this method should not return anything(not even false)
 * and the node will not show up on the dashboard.
 *
 * @param $node
 * The node object on which this check is being performed.
 *
 * @param $target_drupal_locale
 * The target language code is only passed if the Target Language filter is selected on the dashboard.
 *
 */

function module_transperfect_is_node_translatable($node, $drupal_target_locale = NULL) {
  if ($node->type == 'article') {
    return TRUE;
  }
  elseif ($drupal_target_locale != NULL && $drupal_target_locale == 'fr') {
    return TRUE;
  }
}

/*
 * By implementing this hook, you can programmatically filter out if a particular
 * field is translatable or not.
 * If the field is not translatable this method should not return anything(not even false).
 * This hook overrides the fields configuration settings on the Field Configuration tab.
 *
 * @param $node
 * The node object on which this check is being performed.
 *
 * @param $field
 * The field name on which this check is being performed.
 *
 * @param $target_arr
 * The target language array as selected while creating the submission.
 *
 */

function module_transperfect_is_field_translatable($node, $field, $target_arr = NULL) {
  if ($node->type == 'article') {
    if ($field == 'title' || $field == 'body') {
      return TRUE;
    }
  }
  elseif ($target_arr != NULL) {
    foreach ($target_arr as $language) {
      if ($language == 'fr') {
        if ($field == 'title') {
          return TRUE;
        }
      }
    }
  }
}

/*
 * By implementing this hook, you can modify translated node before its imported in Drupal.
 * This hook is called just before the translated node is saved in the database.
 *
 * @param $source_nid
 * The node id of the source node object
 *
 * @param $target_node
 * The translated node object about to be saved in database.
 * Must add '&' to the $target_node for accessing by reference.
 *
 */

function module_transperfect_import_translation($source_nid, &$target_node) {
  $source_node = node_load($source_nid);
  $target_node->title = 'TEST ' . $source_node->title;
}

/*
 * By implementing this hook, you can programmatically filter out if a particular
 * node is translatable or not for a particular language.
 * If the node is not translatable this method should not return anything(not even false)
 * This hook is called just before the content is being send for translation.
 *
 * @param $source_node
 * The source node object
 *
 * @param $drupal_target_locale
 * Drupal target language selected by user for which this node is being translated
 *
 */

function module_transperfect_translate_node_for_language($source_node, $drupal_target_locale) {
  if ($source_node->type == 'article' && $drupal_target_locale == 'fr') {
    return TRUE;
  }
}
