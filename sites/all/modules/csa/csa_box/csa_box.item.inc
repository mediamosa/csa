<?php
// $Id$

/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
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
 * This code handles the selection of assets in user, type scope.
 */

/**
 * Called from jquery.
 *
 * Called when item in the listing is checked.
 *
 * @param $type
 *  inbox / outbox
 * @param $item_id
 *  mediafile_id (for now)
 */
function _csa_box_item_select($type, $item_id) {

  if (_csa_box_check_type($type, FALSE)) {
    _csa_box_item_set_selected($type, array($item_id));
  }

  // We are returning json.
  drupal_set_header('Content-Type: application/json');

  print drupal_to_js(TRUE);
  exit();
}

/**
 * Called from jquery.
 *
 * Called when item in the listing is unchecked.
 *
 * @param $type
 *  inbox / outbox
 * @param $item_id
 *  mediafile_id (for now)
 */
function _csa_box_item_unselect($type, $item_id) {

  if (_csa_box_check_type($type, FALSE)) {
    _csa_box_item_unset_selected($type, array($item_id));
  }

  // We are returning json.
  drupal_set_header('Content-Type: application/json');

  print drupal_to_js(TRUE);
  exit();
}

/**
 * Called by jquery to select a range.
 *
 * @param string $type
 */
function _csa_box_item_select_all($type) {

  $item_ids = empty($_POST['items']) ? array() : explode('|', $_POST['items']);

  // Select them all.
  _csa_box_item_set_selected($type, $item_ids);

  // We are returning json.
  drupal_set_header('Content-Type: application/json');

  print drupal_to_js(TRUE);
  exit();
}

/**
 * Called by jquery to unselect a range.
 *
 * @param string $type
 */
function _csa_box_item_unselect_all($type) {

  $item_ids = empty($_POST['items']) ? array() : explode('|', $_POST['items']);

  // Select them all.
  if (!empty($item_ids)) {
    _csa_box_item_unset_selected($type, $item_ids);
  }

  // We are returning json.
  drupal_set_header('Content-Type: application/json');

  print drupal_to_js(TRUE);
  exit();
}

/**
 * Redirect to fix problems with running JS on background.
 *
 * @param string $type
 */
function _csa_box_item_clearselection($type) {

  unset($_REQUEST['destination']);

  drupal_goto('csa/box/clearselection/' . $type . '/real');
}

/**
 * Clears the current selection.
 *
 * @param string $type
 */
function _csa_box_item_clearselection_real($type) {

  global $user;

  // Clear selection.
  db_query("DELETE FROM csa_box_selection WHERE uid = %d AND type = '%s'", $user->uid, $type);

  // Go back.
  drupal_goto('csa/' . $type);
}

/**
 * Redirect to fix problems with running JS on background.
 *
 * @param string $type
 */
function _csa_box_item_allselection($type) {

  unset($_REQUEST['destination']);

  drupal_goto('csa/box/allselection/' . $type . '/real');
}

/**
 * Clears the current selection.
 *
 * @param string $type
 */
function _csa_box_item_allselection_real($type) {

  global $user;

  if (_csa_box_check_type($type, FALSE)) {
    // Clear selection first.
    db_query("DELETE FROM csa_box_selection WHERE uid = %d AND type = '%s'", $user->uid, $type);

    $options['hook_process'] = '_csa_box_item_fill_selection_' . $type;
    $options['limit'] = 0; // do all.

    // Use hook to fill the selection.
    _csa_box_get_asset_listing($type, $options);
  }

  // Go back.
  drupal_goto('csa/' . $type, 'do_search=1');
}

/**
 * Hook to fill asset_ids for selection.
 *
 * @param array $asset_ids
 */
function _csa_box_item_fill_selection_unpublished($asset_ids) {
  _csa_box_item_set_selected(CSA_BOX_TYPE_UNPUBLISHED, $asset_ids);
}

/**
 * Hook to fill asset_ids for selection.
 *
 * @param array $asset_ids
 */
function _csa_box_item_fill_selection_published($asset_ids) {
  _csa_box_item_set_selected(CSA_BOX_TYPE_PUBLISHED, $asset_ids);
}

/**
 * Hook to fill asset_ids for selection.
 *
 * @param array $asset_ids
 */
function _csa_box_item_fill_selection_search($asset_ids) {
  _csa_box_item_set_selected(CSA_BOX_TYPE_SEARCH, $asset_ids);
}

/**
 * Returns true or false if item was selected.
 *
 * @param string $item_id
 */
function _csa_box_item_is_selected($type, $item_ids) {

  global $user;

  if (empty($item_ids)) {
    return array();
  }

  if (!is_array($item_ids)) {
    $item_ids = array($item_ids);
  }

  $ids = array(); // ID => TRUE/FALSE.
  foreach ($item_ids as $item_id) {
    $items_escaped[] = db_escape_string($item_id);
    $ids[$item_id] = FALSE;
  }

  // Do the query.
  $result = db_query("SELECT asset_id FROM csa_box_selection WHERE uid = %d AND type = '%s' AND asset_id IN('" . implode("','", $items_escaped) . "')", $user->uid, $type);
  $row = db_fetch_array($result);
  while ($row) {
    $ids[$row['asset_id']] = TRUE;
    $row = db_fetch_array($result);
  }

  return $ids;
}

/**
 * Set a set of selection.
 */
function _csa_box_item_set_selected($type, $item_ids) {

  global $user;

  // Insert per 100.
  while (!empty($item_ids)) {

    $max = 100;
    $values = array();
    while ($max-- && !empty($item_ids)) {
      $values[] = '(' . db_escape_string($user->uid) . ",'" . db_escape_string($type) . "','" . db_escape_string(array_shift($item_ids)) . "')";
    }

    db_query('INSERT IGNORE INTO csa_box_selection VALUES' . implode(',', $values));
  }
}

/**
 * Set a unset of selection.
 */
function _csa_box_item_unset_selected($type, $item_ids = array(), $unset_all_when_empty = FALSE) {

  global $user;

  if (empty($item_ids) && $unset_all_when_empty) {
    db_query("DELETE FROM csa_box_selection WHERE uid = %d AND type = '%s'", $user->uid, $type);
  }

  // Delete per 100.
  while (!empty($item_ids)) {
    $max = 100;
    $values = array();
    while ($max-- && !empty($item_ids)) {
      $values[] = "'" . db_escape_string(array_shift($item_ids)) . "'";
    }

    db_query("DELETE FROM csa_box_selection WHERE uid = %d AND type = '%s' AND asset_id IN(" . implode(',', $values) . ')', $user->uid, $type);
  }
}

/**
 * Returns the number of items.
 */
function _csa_box_item_number_of_selected($type) {

  global $user;

  return db_result(db_query("SELECT COUNT(*) FROM csa_box_selection WHERE uid = %d AND type = '%s'", $user->uid, $type));
}

/**
 * Returns the first item in selected.
 *
 * @param array $type
 * @param integer $count
 *   Number of items to return.
 */
function _csa_box_item_next($type, $count = 1) {
  assert($count > 0);

  global $user;

  // Do query with limit.
  $result = db_query_range("SELECT asset_id FROM csa_box_selection WHERE uid = %d AND type = '%s'", $user->uid, $type, 0, $count);

  $ids = array();
  $row = db_fetch_array($result);
  while ($row) {
    $ids[] = $row['asset_id'];
    $row = db_fetch_array($result);
  }

  return count($ids) == 1 ? reset($ids) : $ids;
}

/**
 * returns the text of number of selected items.
 */
function _csa_box_item_get_selected_text($type) {
  $total_selected = _csa_box_item_number_of_selected($type);

  if (!$total_selected) {
    return '<b>' . t('You have not selected any assets, to select all items on all pages, click !here.', array('!here' => l(t('here'), 'csa/box/allselection/' . $type, array('query' => 'destination=csa/' . $type)))) . '</b>';
  }

  return '<b>' . format_plural($total_selected, 'You have selected 1 asset, to clear selection click !here. To select all items on all pages, click !here2.', 'You have selected @count assets, to clear selection click !here. To select all items on all pages, click !here2.', array('!here' => l(t('here'), 'csa/box/clearselection/' . $type, array('query' => 'destination=csa/' . $type)), '!here2' => l(t('here'), 'csa/box/allselection/' . $type, array('query' => 'destination=csa/' . $type)))) . '</b>';
}
