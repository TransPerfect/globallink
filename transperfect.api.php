<?php

/**
 * @file
 * This file provides an example for the
 * implementation of hooks that other modules can implement.
 * Please read documentation for more details.
 */

/**
 * Implements hook_is_node_translatable().
 *
 * @param object $node
 *   The node object on which this check is being performed.
 * @param string $target_drupal_locale
 *   The target language code is only passed if the target language filter is selected on the dashboard.  Defaults to NULL.
 *
 * @return bool
 *   TRUE if the node is translatable.
 */
function module_transperfect_is_node_translatable($node, $drupal_target_locale = NULL) {
  if ($node->type == 'article') {
    return TRUE;
  }
  elseif ($drupal_target_locale != NULL && $drupal_target_locale == 'fr') {
    return TRUE;
  }
}

/**
 * Implements hook_is_field_translatable().
 *
 * @param object $node
 *   The node object on which this check is being performed.
 * @param string $field
 *   The field name on which this check is being performed.
 * @param array $target_arr
 *   The target language array as selected while creating the submission.  Defaults to NULL.
 *
 * @return bool
 *   TRUE if the field is translatable.
 */
function module_transperfect_is_field_translatable($node, $field, $target_arr = NULL) {
  if ($node->type == 'article') {
    if ($field == 'title' || $field == 'body') {
      return TRUE;
    }
  }
  else {
    if ($target_arr != NULL) {
      foreach ($target_arr as $language) {
        if ($language == 'fr') {
          if ($field == 'title') {
            return TRUE;
          }
        }
      }
    }
  }
}

/**
 * Implements hook_import_translation().
 *
 * @param $source_nid
 *   The node id of the source node object.
 * @param $target_node
 *   The translated node object about to be saved in database.
 */
function module_transperfect_import_translation($source_nid, &$target_node) {
  $source_node = node_load($source_nid);
  $target_node->title = 'TEST ' . $source_node->title;
}

/**
 * Implements hook_translate_node_for_language().
 *
 * @param object $source_node
 *   The source node object.
 * @param string $drupal_target_locale
 *   Drupal target language selected by user for which this node is being translated.
 *
 * @return bool
 *   TRUE if the translation was successful.
 */
function module_transperfect_translate_node_for_language($source_node, $drupal_target_locale) {
  if ($source_node->type == 'article' && $drupal_target_locale == 'fr') {
    return TRUE;
  }
}
