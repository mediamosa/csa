<?php
/**
 * CSA is open source Software.
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * CSA is developed for the open source Drupal platform (http://drupal.org).
 * CSA has been developed by Madcap BV (http://www.madcap.nl).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * This module contains all shared helper functions of the CSA application.
 */

/**
 * Prevent the administrator to CSA user menu buttons.
 */
function _csa_access_only_for_csa_user() {
  global $user;

  // If we are not super user, then do normal access check.
  if ($user->uid != 1) {
    return user_access(CSA_PERMISSION_USER);
  }

  $url = url('admin/build/menu-customize/primary-links');
  $url_now = '/' . implode('/', arg());

  // If our url part of the menu edit administration, then we need to access
  // the menu.
  if (drupal_substr($url_now, 0, drupal_strlen($url)) == $url) {
    return TRUE;
  }

  return FALSE;
}

/**
 * Returns MENU_[DEFAULT_]LOCAL_TASK or MENU_CALLBACK if CSA has its links as
 * primary setup. Used for CSA menu hooks.
 */
function _csa_has_primary_block($default_local_task = FALSE) {

  return (variable_get('csa_has_primary_block', FALSE) ? MENU_CALLBACK : ($default_local_task ? MENU_DEFAULT_LOCAL_TASK : MENU_LOCAL_TASK));
}


/**
 * Home page.
 */
function _csa_home() {

  // Get current status of your login.
  $mediamosa_status = _csa_mediamosa_status();

  // No login?
  if (!$mediamosa_status) {
    drupal_set_message(t('You are unable to connect to MediaMosa, please contact your administrator.'), 'error');
  }

  return 'Some page about the user and his options.';
}

/**
 * Helper function for cleaning up strings for display in the browser.
 */
function _csa_name2title($name) {

  $convert_to = array(
    'hasformat' => 'has format',
    'isreferencedby' => 'is referenced by',
    'isformatof' => 'is format of',
    'videotimestamp' => 'video timestamp',
    'videotimestampmodified' => 'video modified',
    'app_id_search' => 'app id',
    'title_alternative' => 'alternative title',
  );

  return drupal_ucfirst(str_replace(array('_', '-'), ' ', isset($convert_to[$name]) ? $convert_to[$name] : $name));
}

/**
 * This function returns a trimmed string, optionally in an HTML <span> tag
 */
function _csa_trim_string($text, $length = FALSE, $html = TRUE, $empty = FALSE) {
  if ($length && drupal_strlen($text) > $length) {
    $text = htmlspecialchars($text);
    if ($html) {
      return sprintf('<span title="%s">%s</span>', $text, drupal_substr($text, 0, $length - 1) . '&hellip;');
    }
    else {
      return htmlspecialchars_decode(drupal_substr($text, 0, $length - 1) . '...');
    }
  }

  if (drupal_strlen(trim($text)) < 1) {
    $text = $empty; //($empty === FALSE) ? t("n/a") : $empty;
  }

  if ($html) {
    return htmlspecialchars($text);
  }

  return $text;
}

/**
 * This function checks the given field and returns TRUE if the field is only
 * allowed once.
 */
function _csa_metadata_field_is_single($field) { // TODO: official single fields???
  $single_fields = array(
    'videotimestamp',
    'videotimestampmodified',
    'created',
    'changed',
    'issued',
    'date',
    'language',
    'title', // questionable?
    'type', // questionable?
  );
  return (in_array($field, $single_fields));
}

/**
 * This function checks the given field and returns TRUE if the field is a date
 * field.
 */
function _csa_field_is_date($field) {
  $date_fields = array(
    'videotimestamp',
    'videotimestampmodified',
    'created',
    'changed',
    'issued',
    'date',
    'play_restriction_start',
    'play_restriction_end',
  );
  return (in_array($field, $date_fields));
}

/**
 * This function returns a themed pager
 */
function _csa_pager($page, $item_count, $item_limit, $element = 0) {
  global $pager_page_array, $pager_total, $pager_total_items;

  $pager_page_array = array($element => $page);
  $pager_total_items[$element] = $item_count; // # of items.

  if (!is_array($element)) {
    $pager_total[$element] = ceil($item_count / $item_limit); // # of links.
  }

  return theme(
    'pager',
    array(), // An array of labels for the controls in the pager.
    $item_limit, // The number of query results to display per page.
    $element, // An optional integer to distinguish between multiple pagers on one page.
    array(), // An associative array of query string parameters to append to the pager links.
    5 // The maximum number of pages in the list.
  );
}


/**
 * Helper function for adding rows to a table
 */
function _csa_table_add_row($key, $value, &$rows, $show_empty = FALSE) {
  if ($show_empty || $value != '') {
    if (_csa_field_is_date($key) && strpos($value, ' 00:00:00') !== FALSE) { // strip time if not set
      list($value, $null) = explode(' ', $value);
    }
    $rows[] = array(
      array('data' => t(_csa_name2title($key)), 'class' => ''),
      array('data' => nl2br(_csa_trim_string($value)))
    );
  }
}

/**
 * Helper function for validating and correcting metadata values.
 */
function _csa_validate_value($key, $value, $form_element = FALSE) {

  if (_csa_field_is_date($key)) {
    $date = '[0-9]{4}\-[0-9]{2}\-[0-9]{2}';
    $datetime = $date . ' [0-9]{2}:[0-9]{2}:[0-9]{2}';
    $valid = FALSE;
    switch (drupal_strlen($value)) {
      case 10: // Date only
        $valid = (preg_match('/^'. $date .'$/', $value) === 1);
        $value .= ' 00:00:00';
        break;
      case 19: // Date + time
        $valid = (preg_match('/^'. $datetime .'$/', $value) === 1);
        break;
    }

    if ($valid === FALSE) {
      form_set_error(($form_element) ? $form_element : $key, t('Date validation failed! "YYYY-MM-DD" or "YYYY-MM-DD HH:MM:SS"'));
    }
  }

  if ($key == 'language') {
    $a_language_codes = explode(',', 'aa,ab,ae,af,ak,am,an,ar,as,av,ay,az,ba,be,bg,bh,bi,bm,bn,bo,br,bs,ca,ce,ch,co,cr,cs,cu,cv,cy,da,de,dv,dz,ee,el,en,eo,es,et,eu,fa,ff,fi,fj,fo,fr,fy,ga,gd,gl,gn,gu,gv,ha,he,hi,ho,hr,ht,hu,hy,hz,ia,id,ie,ig,ii,ik,io,is,it,iu,ja,jv,ka,kg,ki,kj,kk,kl,km,kn,ko,kr,ks,ku,kv,kw,ky,la,lb,lg,li,ln,lo,lt,lu,lv,mg,mh,mi,mk,ml,mn,mo,mr,ms,mt,my,na,nb,nd,ne,ng,nl,nn,no,nr,nv,ny,oc,oj,om,or,os,pa,pi,pl,ps,pt,qu,rm,rn,ro,ru,rw,ry,sa,sc,sd,se,sg,sh,si,sk,sl,sm,sn,so,sq,sr,ss,st,su,sv,sw,ta,te,tg,th,ti,tk,tl,tn,to,tr,ts,tt,tw,ty,ug,uk,ur,uz,ve,vi,vo,wa,wo,xh,yi,yo,za,zh,zu'); // ISO 639-1
    if (!in_array(drupal_strtolower($value), $a_language_codes)) {
      form_set_error(($form_element) ? $form_element : $key, t('Language validation failed!<br />Supported ISO 639-1 languages: %lang.', array('%lang' => implode(', ', $a_language_codes))));
    }
  }

  return $value;
}

/**
 * Check and validate a datestamp string
 * returns FALSE or valid string to use
 *
 * @param string $str
 * @return string
 */
function _csa_check_datestamp($str_date) {

  $str_date = trim($str_date);
  if ($str_date == '') {
    return '0000-00-00 00:00:00'; // Empty is ok
  }

  if (!preg_match('/^\d{4}-\d{2}-\d{2}( \d{2}:\d{2}:\d{2})?$/', $str_date)) {
    return FALSE;
  }

  if (strlen($str_date) == 10) {
    $str_date .= ' 00:00:00';
  }

  list($year, $month, $day, $hour, $minute, $second) = preg_split('/[:\- ]{1}/', $str_date);

  if (!checkdate($month, $day, $year)) {
    return FALSE;
  }

  if ($hour > 23 || $minute > 59 || $second > 59) {
    return FALSE;
  }

  return $str_date;
}

/**
 * Convert YYYY-MM-DD HH:MM:SS to number of seconds since Jan. 1st 1970.
 *
 * Takes date as it is, does not convert to any timezone.
 *
 * @param string $iso8601_date_str
 */
function _csa_iso8601date_to_unix($iso8601_date_str) {
  // Parse the date.
  $date = date_parse($iso8601_date_str);

  // Get current timezone
  $tz = date_default_timezone_get();

  // Set to GMT/UTC so we can convert without timezone problems (take as-is).
  date_default_timezone_set('UTC');

  // Convert to integer.
  $time = gmmktime($date['hour'], $date['minute'], $date['second'], $date['month'], $date['day'], $date['year']);

  // Back to original timezone, so we dont screw with other time functions
  date_default_timezone_set($tz);

  return $time;
}

/**
 * Will use datetime stamp instead of unix time.
 *
 * Does not convert date to timezone, take as-is.
 *
 * @param datetime $date
 */
function _csa_format_date($date, $timezone = 0) {
  return format_date(_csa_iso8601date_to_unix($date), 'medium', '', $timezone);
}

/**
 * Returns the value for the form.
 *
 * @param array $form_values
 * @param string $name
 */
function _csa_form_value($form_values, $name) {
  return $form_values[$name];
}

/**
 * Mediamosa connector.
 */
function _csa_mediamosa_connector($reset_connection = FALSE) {
  global $user;

  // Store for reuse.
  static $mediamosa_connectors = array();

  // Must be logged in.
  if (empty($user->uid) || !is_numeric($user->uid)) {
    return FALSE;
  }

  // Get connection.
  $app_name = _csa_get_user_value_connection($user, 'app_name');
  $shared_key = _csa_get_user_value_connection($user, 'shared_key');
  $mediamosa_url = _csa_get_user_value_connection($user, 'mediamosa_url');

  // If super user, then use the main connection unless its user settings were
  // set.
  if ($user->uid == 1 && (empty($app_name) || empty($shared_key) || empty($mediamosa_url))) {
    $app_name = variable_get('mediamosa_connector_username', _csa_get_user_value_connection($user, 'app_name'));
    $shared_key = variable_get('mediamosa_connector_password', _csa_get_user_value_connection($user, 'shared_key'));
    $mediamosa_url = variable_get('mediamosa_connector_url', _csa_get_user_value_connection($user, 'mediamosa_url'));
  }

  // Get the connection.
  if (
    !empty($app_name) &&
    !empty($shared_key) &&
    !empty($mediamosa_url)
  ) {
    // If we already have the connector, reuse it.
    if (!empty($mediamosa_connectors[$app_name][$shared_key][$mediamosa_url])) {
      return $mediamosa_connectors[$app_name][$shared_key][$mediamosa_url];
    }

    // Create it.
    $mediamosa_connectors[$app_name][$shared_key][$mediamosa_url] = new mediamosa_connector(
      $app_name,
      $shared_key,
      $mediamosa_url
    );

    // Reset possible cookie when needed.
    if ($reset_connection) {
      $mediamosa_connectors[$app_name][$shared_key][$mediamosa_url]->reset_session($app_name, $shared_key, $mediamosa_url);
    }

    return $mediamosa_connectors[$app_name][$shared_key][$mediamosa_url];
  }

  // No connector available.
  return FALSE;
}

/**
 * Mediamosa request.
 */
function _csa_mediamosa_request($url, array $options = array(), $check = FALSE) {
  $result = FALSE;

  // Get the connection class object.
  $mediamosa_connector = _csa_mediamosa_connector();

  // Do the request.
  if ($mediamosa_connector) {
    $result = $mediamosa_connector->request($url, $options);
    if ($check) {
      mediamosa_connector::check_result($result, FALSE, '%message');
    }
  }

  return $result;
}

/**
 * Same as _csa_mediamosa_request, but will check result.
 */
function _csa_mediamosa_request_check($url, array $options = array()) {
  return _csa_mediamosa_request($url, $options, TRUE);
}

/**
 * Same as _csa_mediamosa_request but always POST.
 */
function _csa_mediamosa_request_post($url, array $options = array(), $check = FALSE) {
  $options['method'] = 'POST';
  return _csa_mediamosa_request($url, $options, $check);
}

/**
 * Same as _csa_mediamosa_request_post, but will check result.
 */
function _csa_mediamosa_request_post_check($url, array $options = array()) {
  return _csa_mediamosa_request_post($url, $options, TRUE);
}

/**
 * Check response and sets drupal message when needed, returns TRUE / FALSE.
 */
function _csa_mediamosa_check_result($response, $ok = NULL, $error = NULL) {
  return mediamosa_connector::check_result($response, $ok, $error);
}

/**
 * Check if the current user can login onto mediamosa.
 *
 * returns an array of information, because in future we might want to know if
 * the user can upload files etc.
 *
 * returns FALSE when login is not available.
 */
function _csa_mediamosa_status($reset = FAlSE) {
  global $user;

  static $csa_status = NULL;

  // Remove it.
  if ($reset) {
    unset($csa_status);
    return;
  }

  if (!empty($csa_status)) {
    return $csa_status;
  }

  // Reset login when the last try failed.
  if (empty($csa_status)) {
    _csa_mediamosa_connector(TRUE);
  }

  // Get the status.
  $result = _csa_mediamosa_request('version');

  $csa_status = FALSE;
  if ($result) {
    $csa_status['version'] = (array)reset($result->xml->items);

    return $csa_status;
  }
  else {
    // Warning in the watchdog.
    _csa_log_warning("The Mediamosa connection of the user '@user' doesn't work", array('@user' => $account->name));
  }

  return $csa_status;
}

/**
 * Helper to get value from user object.
 *
 * @param $account
 *  The user account.
 * @param string $name
 *  The name of the field.
 * @param string $area
 *  The array where the data is stored in user object.
 * @param $area
 *  Where to store.
 */
function _csa_get_user_value($account, $name, $default_value = '', $area = 'csa_prefs') {
  global $user;

  // If not provided, use user.
  if (empty($account)) {
    $account = $user;
  }

  return (isset($account->{$area}) && isset($account->{$area}[$name])) ? $account->{$area}[$name] : $default_value;
}

/**
 * Shortcut for csa_prefs data.
 */
function _csa_get_user_value_prefs($account, $name, $default_value = '') {
  return _csa_get_user_value($account, $name, $default_value, 'csa_prefs');
}

/**
 * Shortcut for csa_connetion data.
 */
function _csa_get_user_value_connection($account, $name, $default_value = '') {
  return _csa_get_user_value($account, $name, $default_value, 'csa_connection');
}

/**
 * Returns the user id of the current csa admin.
 *
 * @FIXME: should we generate or store this ID somewhere? We need this ID to
 * edit and store assets and mediafiles. We could make it unique per login and
 * do queries based on the owner.
 */
function _csa_get_mediamosa_user_id() {
  global $user;

  return _csa_get_user_value_connection($user, 'mediamosa_user', 'csa.admin');
}

/**
 * This user ID is fixed for system objects like collections we use for
 * selection.
 *
 * @return string
 */
function _csa_get_mediamosa_admin_user_id() {
  return 'csa.admin';
}
/**
 * Returns the group ID to use for upload. We use this ID to set the collection
 * group ID. Use this ID only for that(!).
 */
function _csa_get_mediamosa_upload_group_id() {
  return 'csa.upload.group';
}

/**
 * Get the current quota info from the user.
 *
 * We store the quota in the user. Will need to refresh quota when we think
 * it has changed.
 */
function _csa_user_get_quota($user_id = NULL, $refresh = FALSE) {

  // Get user_id.
  $user_id = empty($user_id) ? _csa_get_mediamosa_user_id() : $user_id;

  // Get quota.
  $result = mediamosa_connector::response2array(_csa_mediamosa_request('user/' . $user_id));

  // User does not exists, create.
  if (empty($result['header']['request_result_id']) || $result['header']['request_result_id'] != 601) {
    $options['data'] = array(
      'user' => $user_id,
      'quotum' => 0,
    );

    // Create user.
    mediamosa_connector::response2array(_csa_mediamosa_request_post('user/create', $options));

    // Get quota.
    $result = mediamosa_connector::response2array(_csa_mediamosa_request('user/' . $user_id));
  }

  $user_quota = array(
    'total' => 0, // in mb.
    'used' => 0,
    'free' => 0,
  );

  if (!empty($result['items']['item'])) {
    $item = reset($result['items']['item']);

    // Total quota available.
    $user_quota['total'] = (int)$item['app_quota_mb'] * 1024;
    $user_quota['free'] = (int)$item['quota_available_mb'] * 1024;
    $user_quota['used'] = $user_quota['total'] - $user_quota['free'];
  }

  // Set the text.
  $user_quota['text'] = $user_quota['total'] > 0 ? t('Quota for upload; total: @total, used: @used, <b>free: @free</b>.', array('@total' => format_size($user_quota['total'] * 1024), '@used' => format_size($user_quota['used'] * 1024), '@free' => format_size($user_quota['free'] * 1024))) : t('No user quota information available.');

  return $user_quota;
}

/**
 * Check if input is realm.
 */
function _csa_is_realm($domain_realm) {
  return ((strpos($domain_realm, '@') === FALSE) ? FALSE : TRUE);
}

/**
 * Check if input is domain.
 */
function _csa_is_domain($domain_realm) {
  // If its a realm, its not a domain and visa versa.
  return !_csa_is_realm($domain_realm) && !empty($domain_realm);
}

/**
 * Get the users transcodings.
 */
function _csa_get_transcodings($account) {
  return _csa_app_transcode_list();
}

/**
 * Get the users selection of applications.
 */
function _csa_get_applications($account) {
  return _csa_app_get_list();
}

/**
 * Get list upload collections;
 *  id => title.
 */
function _csa_get_upload_collections($fields = array('title')) {

  static $collections = NULL;

  if (isset($collections)) {
    return $collections;
  }

  // Offset, limit etc.
  $options['offset'] = 0;
  $options['limit'] = 100;
  $options = array('group_ids' => array(_csa_get_mediamosa_upload_group_id()));

  // Get the collections.
  $result = _csa_collection_search($options);
  $collections = array();

  foreach ($result['collections'] as $coll_id => $collection) {
    unset($collection['@attributes']);

    // If single item field, then don't store as array, no items mean all.
    if (empty($fields)) {
      $collections[$coll_id] = $collection;
    }
    elseif (count($fields) == 1) {
      $collections[$coll_id] = $collection[reset($fields)];
    }
    else {
      $data = array();
      foreach ($fields as $field) {
        $data[$field] = $collection[$field];
      }

      $collections[$coll_id] = $data;
    }
  }

  return $collections;
}

/**
 * Get collection list for drop down list.
 */
function _csa_get_upload_collections_for_select() {
  // Get current created upload collections.
  $upload_collections = _csa_get_upload_collections();

  // Default name collection.
  $default_name = _csa_get_default_collection_name();

  // Search and remove default so we can put it at first entry.
  $default_name_id = $default_name;

  foreach ($upload_collections as $id => $title) {
    if (drupal_strtolower($title) == drupal_strtolower($default_name)) {
      unset($upload_collections[$id]);
      $default_name_id = $id;
    }
  }

  // Sort.
  asort($upload_collections);

  // Done.
  return array_merge(array($default_name_id => $default_name), $upload_collections);
}

/**
 * Returns the global settings for required metadata.
 */
function _csa_settings_required_metadata_get() {
  $default = array(); // You can add forced defaults here.

  // Default is based on default setup.
  $csa_settings_required_metadata = variable_get('csa_settings_required_metadata', array());

  if (empty($csa_settings_required_metadata)) {
    $metadata = _csa_asset_metadata_list_misc();

    foreach ($metadata['metadata'] as $group_name => $group_metadata) {
      foreach ($group_metadata as $name => $props) {
        if (in_array($name, $default)) {
          $csa_settings_required_metadata[$props['prop_id']] = $metadata['metadata_propid_to_title'][$props['prop_id']];
        }
      }
    }
  }

  return $csa_settings_required_metadata;
}

/**
 * Set the global settings for required metadata.
 *
 * @param array $csa_settings_required_metadata
 */
function _csa_settings_required_metadata_set($csa_settings_required_metadata) {
  variable_set('csa_settings_required_metadata', $csa_settings_required_metadata);
}

/**
 * Returns a list of metadata that can be entered double.
 */
function _csa_settings_multiple_metadata_get() {
  return variable_get('csa_settings_multiple_metadata', array());
}

/**
 * Returns a list of metadata that can be entered double.
 */
function _csa_settings_multiple_metadata_set($csa_settings_multiple_metadata) {
  variable_set('csa_settings_multiple_metadata', $csa_settings_multiple_metadata);
}

/**
 * Return the destination.
 */
function _csa_drupal_get_destination() {
  if (isset($_REQUEST['default_destination'])) {
    return array('destination' => $_REQUEST['default_destination']);
  }

  list($name, $destination) = explode('=', drupal_get_destination(), 2);
  return array($name => urldecode($destination));
}
