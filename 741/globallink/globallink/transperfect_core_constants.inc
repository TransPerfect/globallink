<?php

define('TPT_ROLE_MANAGE_TRANSLATIONS', t('can manage GlobalLink translation tasks and settings'));
define('TPT_ROLE_DO_TRANSALTIONS', t('can manage GlobalLink translation tasks'));
define('TPT_ROLE_VIEW_TRANSLATIONS', t('can view GlobalLink translation tasks'));

define('TPT_STATUS_PENDING_TRANSLATIONS', 'Pending Translations');
define('TPT_STATUS_SENT_FOR_TRANSLATIONS', 'Sent for Translations');
define('TPT_STATUS_PUBLISHED', 'Published');
define('TPT_STATUS_COMPLETED', 'Translation Completed');
define('TPT_STATUS_DELETED', 'Node Deleted');
define('TPT_STATUS_SOURCE_DELETED', 'Source Deleted');
define('TPT_STATUS_ERROR', 'Error');
define('TPT_STATUS_ARCHIVED', 'Archived');
define('TPT_STATUS_CANCELLED', 'Cancelled');

define('PD_STATUS_CANCELLED', 'CANCELLED');

define('TPT_LABEL_ACTIVE_SUBMISSION', t('Active Submission Name'));
define('TPT_LABEL_SUBMISSION', t('Submission'));
define('TPT_LABEL_CONTENT', t('Content'));
define('TPT_LABEL_CONTENT_TYPE', t('Content Type'));
define('TPT_LABEL_SOURCE_LANG', t('Source Language'));
define('TPT_LABEL_TARGET_LANG', t('Target Language'));
define('TPT_LABEL_STATUS', t('Status'));
define('TPT_LABEL_LAST_UPDATED', t('Last Updated'));
define('TPT_LABEL_TITLE', t('Title'));
define('TPT_LABEL_FIELD_MACHINE', t('Field Name'));
define('TPT_LABEL_FIELD_LABEL', t('Field Label'));
define('TPT_LABEL_FIELD_TYPE', t('Field Type'));

define('TPT_BTN_DASHBOARD_GO', ' ' . t('Go') . ' ');
define('TPT_BTN_DASHBOARD_BACK', ' ' . t('Back') . ' ');
define('TPT_BTN_DASHBOARD_SEND', ' ' . t('Send for Translation') . ' ');
define('TPT_BTN_DASHBOARD_CLEAR_CHANGED_FLAG', ' ' . t('Clear "Changed" Status') . ' ');
define('TPT_BTN_DASHBOARD_CANCEL_SUBMISSION', ' ' . t('Cancel Submission') . ' ');
define('TPT_BTN_DASHBOARD_CANCEL_SELECT_DOCUMENT', ' ' . t('Cancel Selected Documents') . ' ');
define('TPT_BTN_DASHBOARD_CLEAR_CANCELLED_DOCUMENTS', ' ' . t('Clear Cancelled Documents') . ' ');
define('TPT_BTN_DASHBOARD_UPDATE_SELECTED', ' ' . t('Update Selected') . ' ');
define('TPT_BTN_DASHBOARD_UPDATE_ALL', ' ' . t('Update All') . ' ');
define('TPT_BTN_DASHBOARD_SAVE', ' ' . t('Save') . ' ');
define('TPT_BTN_DASHBOARD_CLEAR_DELETED_RECORDS', ' ' . t('Clear Deleted Records') . ' ');
define('TPT_BTN_DASHBOARD_REFRESH', ' ' . t('Refresh Page') . ' ');
define('TPT_BTN_DASHBOARD_FILTER', ' ' . t('Filter') . ' ');
define('TPT_BTN_DASHBOARD_RESET', ' ' . t('Reset') . ' ');
define('TPT_BTN_DASHBOARD_ARCHIVE', ' ' . t('Archive Records') . ' ');
define('TPT_BTN_ADD', ' ' . t('Add') . ' ');
define('TPT_BTN_LOCALE_DELETE', ' ' . t('Delete') . ' ');
define('TPT_BTN_SETTINGS_PD', ' ' . t('Save PD Settings') . ' ');
define('TPT_BTN_SETTINGS_TEST_PD', ' ' . t('Test PD Connectivity') . ' ');
define('TPT_BTN_SETTINGS_SAVE_TEST_PD', ' ' . t('Save and Test Settings') . ' ');
define('TPT_BTN_SETTINGS_GENERAL', ' ' . t('Save Adaptor Settings') . ' ');
define('TPT_BTN_FIELD_UPDATE', ' ' . t('Update') . ' ');

define('TPT_PAGER_LIMIT', variable_get('transperfect_pager_limit', 10));
define('TPT_PAGER_NUMBER', 0);

define('TPT_DASHBOARD_FILTER_TYPE_BY_STATUS', 'status');
define('TPT_DASHBOARD_FILTER_TYPE_BY_TITLE', 'title');
define('TPT_DASHBOARD_FILTER_TYPE_BY_CONTENT_TYPE', 'type');
define('TPT_DASHBOARD_FILTER_TYPE_BY_TARGET_LANG', 'target');

define('TPT_TABLE_LOCALE', 'transperfect_locale');
define('TPT_TABLE_CORE', 'transperfect_core');

define('TPT_LOGGING_CONFIG_DISABLED', '0');
define('TPT_LOGGING_CONFIG_INFO', '1');
define('TPT_LOGGING_CONFIG_DEBUG', '2');

define('TPT_LOGGING_SEVERITY_INFO', 'Info');
define('TPT_LOGGING_SEVERITY_WARNING', 'Warning');
define('TPT_LOGGING_SEVERITY_ERROR', 'Error');
define('TPT_LOGGING_SEVERITY_DEBUG', 'Debug');

define('TPT_OBJECT_TYPE_CONTENT', 'Content');
define('TPT_OBJECT_TYPE_BLOCK', 'Block');
define('TPT_OBJECT_TYPE_FIELDABLE_PANELS', 'Fieldable_Panels_Panes');
define('TPT_OBJECT_TYPE_MENU', 'Menu');
define('TPT_OBJECT_TYPE_TAXONOMY', 'Taxonomy');
define('TPT_OBJECT_TYPE_WEBFORM', 'Webform');
define('TPT_OBJECT_TYPE_INTERFACE', 'Interface');
define('TPT_OBJECT_TYPE_GLOBALLINK', 'GlobalLink');

/**
 * Returns GlobalLink modules root menu
 */
function transperfect_get_root_menu($menu_item) {

  return 'admin/' . $menu_item;
}