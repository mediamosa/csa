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
 * The inbox contains all unpublished assets.
 */

define('CSA_BOX_TYPE_UNPUBLISHED', 'unpublished');
define('CSA_BOX_TYPE_PUBLISHED', 'published');
define('CSA_BOX_TYPE_SEARCH', 'search');

define('CSA_SEARCH_OP_APPLY_AND_SEARCH', 'Apply filter & Show results');
define('CSA_SEARCH_OP_ADD_METADATA', 'Add search item');
define('CSA_SEARCH_OP_RESET_METADATA', 'Reset');

/**
 * Make sure the type is allowed.
 *
 * @param string $type
 * @param bool $fatal
 *   TRUE - throw exception instead of returning boolean.
 *
 * @throws Exception when incorrect ($fatal = TRUE).
 */
function _csa_box_check_type($type, $fatal = TRUE) {
  if (!in_array($type, array(CSA_BOX_TYPE_UNPUBLISHED, CSA_BOX_TYPE_PUBLISHED, CSA_BOX_TYPE_SEARCH))) {
    if ($fatal) {
      throw new Exception(t('Unknown type: @type', array('@type' => $type)));
    }

    return FALSE;
  }

  return TRUE;
}

/**
 * Returns the listing based on the page.
 * Returns a list of asset_ids.
 */
function _csa_box_get_asset_listing($type, $options = array()) {

  // Check the type.
  _csa_box_check_type($type);

  $order = array(
    t('Filename') => 'filename',
    t('Upload date') => 'videotimestamp',
    t('Size') => 'filesize',
  );

  $sort = array(
    'desc' => 'DESC',
    'asc' => 'ASC',
  );

  $order_by = empty($_GET['order']) || empty($order[$_GET['order']]) ? 'videotimestamp' : $order[$_GET['order']];
  $order_direction = empty($_GET['sort']) || empty($sort[drupal_strtolower($_GET['sort'])]) ? 'desc' : $sort[drupal_strtolower($_GET['sort'])];

  $options += array(
    'limit' => CSA_INBOX_ASSETS_PER_PAGE,
    'offset' => 0,
    'hide_empty_assets' => TRUE,
    'return_asset_ids' => TRUE,
    'order_by' => $order_by,
    'order_direction' => $order_direction,
    'show_collections' => FALSE,
    'hook_process' => NULL, // Provide hook to fill with result.
    'coll_id' => array(),
  );

  switch ($type) {
    case CSA_BOX_TYPE_UNPUBLISHED:
      $options['cql'] = 'is_master_slaved = "FALSE"';
      return _csa_asset_search($options);

    case CSA_BOX_TYPE_PUBLISHED:
      $options['cql'] = 'is_master_slaved = "TRUE"';
      return _csa_asset_search($options);

    case CSA_BOX_TYPE_SEARCH:
      $cql = array();

      $values = empty($_SESSION['csa_forms']['search']['values']) ? array() : $_SESSION['csa_forms']['search']['values'];

      $values += array(
        'search_operator' => 'AND',
        'cql' => '',
        'search_collection' => 0,
        'search_published' => 0,
      );

      $values['search_operator'] = ' ' . drupal_strtoupper($values['search_operator']) . ' ';

      if (!empty($values['cql'])) {
        $cql[] = '(' . $values['cql'] . ')';
      }

      if (!empty($values['search_collection'])) {
        $coll_id = array($values['search_collection']);
      }

      switch ($values['search_published']) {
        case 1:
          $cql[] = 'is_master_slaved = "TRUE"';
          break;

        case 2:
          $cql[] = 'is_master_slaved = "FALSE"';
          break;
      }

      // Do metadata search.
      $metadata = _csa_asset_metadata_list_misc();

      if (!empty($_SESSION['csa_forms'][$values['type']]['search_metadata'])) {
        foreach ($_SESSION['csa_forms'][$values['type']]['search_metadata'] as $prop_id => $items) {
          foreach ($items as $key => $item) {
            if (empty($metadata['metadata_propid_to_name'][$prop_id])) {
              continue;
            }

            // Must have value.
            if (empty($_SESSION['csa_forms'][$values['type']]['values']['prop_id_' . $prop_id . '_' . $key])) {
              continue;
            }

            // Get value.
            $value = $_SESSION['csa_forms'][$values['type']]['values']['prop_id_' . $prop_id . '_' . $key];
            $value_till = empty($_SESSION['csa_forms'][$values['type']]['values']['prop_id_' . $prop_id . '_' . $key . '_till']) ? '' : $_SESSION['csa_forms'][$values['type']]['values']['prop_id_' . $prop_id . '_' . $key . '_till'];

            switch (drupal_strtoupper($metadata['metadata_propid_to_type'][$prop_id])) {
              case 'INT':
              case 'DATETIME':
                if (empty($value_till)) {
                  $value_till = $value . drupal_substr('9999-12-31 23:59:59', drupal_strlen($value));
                }

                $value = $value . drupal_substr('0000-01-01 00:00:00', drupal_strlen($value));
                $value_till = $value_till . drupal_substr('0000-01-01 00:00:00', drupal_strlen($value));

                $cql[] = '(' . $metadata['metadata_propid_to_name'][$prop_id] . ' >= "' . $value . '" AND '  . $metadata['metadata_propid_to_name'][$prop_id] . ' <= "' . $value_till . '")';
                break;

              case 'CHAR':
                $cql[] = $metadata['metadata_propid_to_name'][$prop_id] . ' = "' . $value . '"';
                break;
            }
          }
        }
      }

      $cql = implode($values['search_operator'], $cql);

      $options['cql'] = $cql;
      if (!empty($coll_id)) {
        $options['coll_id'] = $coll_id;
      }

      return _csa_asset_search($options);
  }

  // Just in case we forget.
  assert(0);
  return array();
}

/**
 * Returns a list of asset_ids.
 */
function _csa_box_get_asset_listing_selection($options = array()) {

  $options += array(
    'limit' => CSA_INBOX_ASSETS_PER_PAGE,
    'offset' => 0,
  );

  // Do query.
  $result = db_query_range("SELECT asset_id FROM {csa_box_selection} WHERE type = '%s' ORDER BY uid ASC", array(arg(1)), $options['offset'], $options['limit']);

  $asset_ids = array();

  $row = db_fetch_array($result);
  while ($row) {
    $asset_ids[] = $row['asset_id'];
    $row = db_fetch_array($result);
  }

  return $asset_ids;
}

/**
 * The inbox form shows the unpublished assets/mediafiles.
 */
function _csa_inbox_form($form_state) {
  return _csa_box_form($form_state, array('type' => CSA_BOX_TYPE_UNPUBLISHED));
}

/**
 * The outbox form shows the published assets/mediafiles.
 */
function _csa_outbox_form($form_state) {
  return _csa_box_form($form_state, array('type' => CSA_BOX_TYPE_PUBLISHED));
}

/**
 * The inbox form shows the unpublished assets/mediafiles.
 */
function _csa_searchbox_form($form_state) {
  $include_search_results = !empty($form_state['storage']['do_search']) || !empty($_GET['do_search']);
  return _csa_box_form($form_state, array('type' => CSA_BOX_TYPE_SEARCH, 'include_search' => TRUE, 'include_search_results' => $include_search_results));
}

/**
 * Validate function for box form.
 */
function _csa_searchbox_form_validate($form, $form_state) {
}

/**
 * Validate function for box form.
 */
function _csa_searchbox_form_submit($form, $form_state) {
}

/**
 * Upload collection create form.
 *
 * @param array $options
 *  - type (inbox)
 *    inbox / outbox, type of box to show.
 *  - include_transcodings (FALSE)
 *    TRUE / FALSE to enable listing of transcodings.
 *  - include_search (FALSE)
 *    TRUE / FALSE to enable search form.
 *  - include_search_results (TRUE)
 *    TRUE / FALSE to do search and show results.
 */
function _csa_box_form($form_state, $options = array()) {
  $form = array();

  $options += array(
    'type' => CSA_BOX_TYPE_UNPUBLISHED,
    'include_transcodings' => FALSE,
    'include_search' => FALSE,
    'include_search_results' => TRUE,
  );

  if (!empty($options['include_search'])) {
    // Include the search form (direct in $form or else the validate and submit
    // functions will not work..
    $form = _csa_box_search_form($form_state);
  }

  $actions = array();

  switch ($options['type']) {
    case CSA_BOX_TYPE_UNPUBLISHED:
      $actions[CSA_BOX_ACTION_PUBLISH] = t(CSA_BOX_ACTION_PUBLISH);
      $actions[CSA_BOX_ACTION_DELETE] = t(CSA_BOX_ACTION_DELETE);
      $actions[CSA_BOX_ACTION_TRANSCODE] = t(CSA_BOX_ACTION_TRANSCODE);
      $actions[CSA_BOX_ACTION_METADATA] = t(CSA_BOX_ACTION_METADATA);
      $actions[CSA_BOX_ACTION_GENERATE_STILLS] = t(CSA_BOX_ACTION_GENERATE_STILLS);
      $actions[CSA_BOX_ACTION_ADD_TO_COLLECTION] = t(CSA_BOX_ACTION_ADD_TO_COLLECTION);
      $actions[CSA_BOX_ACTION_REMOVE_FROM_COLLECTION] = t(CSA_BOX_ACTION_REMOVE_FROM_COLLECTION);
      $actions[CSA_BOX_ACTION_BROWSE_SELECTION] = t(CSA_BOX_ACTION_BROWSE_SELECTION);
      $actions[CSA_BOX_ACTION_ACCESS_RULES] = t(CSA_BOX_ACTION_ACCESS_RULES);
      break;

    case CSA_BOX_TYPE_PUBLISHED:
      $actions[CSA_BOX_ACTION_UNPUBLISH] = t(CSA_BOX_ACTION_UNPUBLISH);
      $actions[CSA_BOX_ACTION_DELETE] = t(CSA_BOX_ACTION_DELETE);
      $actions[CSA_BOX_ACTION_TRANSCODE] = t(CSA_BOX_ACTION_TRANSCODE);
      $actions[CSA_BOX_ACTION_METADATA] = t(CSA_BOX_ACTION_METADATA);
      $actions[CSA_BOX_ACTION_GENERATE_STILLS] = t(CSA_BOX_ACTION_GENERATE_STILLS);
      $actions[CSA_BOX_ACTION_ADD_TO_COLLECTION] = t(CSA_BOX_ACTION_ADD_TO_COLLECTION);
      $actions[CSA_BOX_ACTION_REMOVE_FROM_COLLECTION] = t(CSA_BOX_ACTION_REMOVE_FROM_COLLECTION);
      $actions[CSA_BOX_ACTION_BROWSE_SELECTION] = t(CSA_BOX_ACTION_BROWSE_SELECTION);
      $actions[CSA_BOX_ACTION_ACCESS_RULES] = t(CSA_BOX_ACTION_ACCESS_RULES);
      break;

    case CSA_BOX_TYPE_SEARCH:
      $actions[CSA_BOX_ACTION_PUBLISH] = t(CSA_BOX_ACTION_PUBLISH);
      $actions[CSA_BOX_ACTION_UNPUBLISH] = t(CSA_BOX_ACTION_UNPUBLISH);
      $actions[CSA_BOX_ACTION_DELETE] = t(CSA_BOX_ACTION_DELETE);
      $actions[CSA_BOX_ACTION_TRANSCODE] = t(CSA_BOX_ACTION_TRANSCODE);
      $actions[CSA_BOX_ACTION_METADATA] = t(CSA_BOX_ACTION_METADATA);
      $actions[CSA_BOX_ACTION_GENERATE_STILLS] = t(CSA_BOX_ACTION_GENERATE_STILLS);
      $actions[CSA_BOX_ACTION_ADD_TO_COLLECTION] = t(CSA_BOX_ACTION_ADD_TO_COLLECTION);
      $actions[CSA_BOX_ACTION_REMOVE_FROM_COLLECTION] = t(CSA_BOX_ACTION_REMOVE_FROM_COLLECTION);
      $actions[CSA_BOX_ACTION_BROWSE_SELECTION] = t(CSA_BOX_ACTION_BROWSE_SELECTION);
      $actions[CSA_BOX_ACTION_ACCESS_RULES] = t(CSA_BOX_ACTION_ACCESS_RULES);
      break;
  }

  // Sort.
  sort($actions);

  // Get upload collections.
  $collections = _csa_get_upload_collections(array());

  $page = isset($_GET['page']) ? $_GET['page'] : 0;
  $limit = CSA_INBOX_ASSETS_PER_PAGE;
  $offset = $page * $limit;

  // Get the assetids from the collections.
  if ($options['type'] == CSA_BOX_TYPE_UNPUBLISHED) {
    $empty = t("You don't have any unpublished assets");
  }
  elseif ($options['type'] == CSA_BOX_TYPE_PUBLISHED) { // Outbox.
    // Get published items.
    $empty = t("You don't have any published assets");
  }
  else { // Search.
    // Get published items.
    $empty = t("You don't have any assets");
  }

  // Save the type in the form.
  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $options['type'],
  );
  $form['include_search_results'] = array(
    '#type' => 'hidden',
    '#value' => $options['include_search_results'],
  );

  if (!empty($options['include_search_results']) || !empty($_GET['include_search_results'])) {
    // Get the assets.
    $assets_ids = _csa_box_get_asset_listing(arg(1), array('offset' => $offset, 'limit' => $limit));

    // Now get the mediafile info.
    $assets = _csa_assets_get(array_keys($assets_ids['assets']));

    $options['page'] = $page;
    $options['item_count'] = $assets_ids['item_count_total'];
    $options['item_limit'] = $limit;
    $options['empty'] = $empty;
    $options['offset'] = $offset;

    if (!empty($assets)) {
      foreach ($actions as $op => $action) {
        $form['csa_submit1'][$op] = array(
          '#type' => 'submit',
          '#value' => $action,
        );
      }
    }

    // Now we got the assets/mediafiles, now fill the information into the form.
    $form['mediafiles']['#theme'] = 'csa_box_list';
    $form['mediafiles']['#tree'] = TRUE;
    $form['mediafiles']['#value'] = array_merge_recursive(array('collections' => $collections), array('assets' => $assets), array('options' => $options));

    if (!empty($assets)) {
      foreach ($actions as $op => $action) {
        $form['csa_submit2'][$op] = array(
          '#type' => 'submit',
          '#value' => $action,
        );
      }

      $form['#validate'][] = '_csa_box_form_validate';
      $form['#submit'][] = '_csa_box_form_submit';
    }
  }

  return $form;
}

/**
 * Validate function for box form.
 */
function _csa_box_form_validate($form, $form_state) {

  // Get the values;
  $values = $form_state['values'];

  // Get pressed button.
  $op = $values['op'];

  // Check type.
  if (!_csa_box_check_type($values['type'], FALSE)) {
    form_set_error('type', t('Invalid type detected.'));
  }

  switch ($op) {
    case t(CSA_SEARCH_OP_APPLY_AND_SEARCH):
    case t(CSA_SEARCH_OP_ADD_METADATA):
    case t(CSA_SEARCH_OP_RESET_METADATA):
      // Ok.
      break;

    case t(CSA_BOX_ACTION_DELETE):
    case t(CSA_BOX_ACTION_TRANSCODE):
    case t(CSA_BOX_ACTION_METADATA):
    case t(CSA_BOX_ACTION_ADD_TO_COLLECTION):
    case t(CSA_BOX_ACTION_REMOVE_FROM_COLLECTION):
    case t(CSA_BOX_ACTION_GENERATE_STILLS):
    case t(CSA_BOX_ACTION_BROWSE_SELECTION):
    case t(CSA_BOX_ACTION_ACCESS_RULES):
      if (!_csa_box_item_number_of_selected($values['type'])) {
        form_set_error('action', t('You must select one or more items.'));
      }
      break;

    case t(CSA_BOX_ACTION_PUBLISH):
      if (!_csa_box_item_number_of_selected($values['type'])) {
        form_set_error('action', t('You must select one or more items.'));
      }

      if (!in_array($values['type'], array(CSA_BOX_TYPE_UNPUBLISHED, CSA_BOX_TYPE_SEARCH))) {
        form_set_error('action', t('Invalid inbox.'));
      }
      break;

    case t(CSA_BOX_ACTION_UNPUBLISH):
      if (!_csa_box_item_number_of_selected($values['type'])) {
        form_set_error('action', t('You must select one or more items.'));
      }

      if (!in_array($values['type'], array(CSA_BOX_TYPE_PUBLISHED, CSA_BOX_TYPE_SEARCH))) {
        form_set_error('action', t('Invalid outbox.'));
      }
      break;

    default:
      form_set_error('action', t('Unknown action or button pressed.'));
      _csa_log_critical('Submit form of @box was not valid!', array('@box' => $values['type']));
      break;
  }
}

/**
 * Submiting the box form.
 */
function _csa_box_form_submit($form, &$form_state) {

  // Get the values;
  $values = $form_state['values'];

  // Get pressed button.
  $op = $values['op'];

  // Depending of the type, we need another confirm or we'll just start.
  switch ($op) {
    case t(CSA_SEARCH_OP_APPLY_AND_SEARCH):
    case t(CSA_SEARCH_OP_ADD_METADATA):
    case t(CSA_SEARCH_OP_RESET_METADATA):
      // Ok.
      break;

    case t(CSA_BOX_ACTION_DELETE):
      drupal_goto('csa/box/delete/confirm/' . $values['type']);

    case t(CSA_BOX_ACTION_PUBLISH):
      drupal_goto('csa/box/publish');

    case t(CSA_BOX_ACTION_UNPUBLISH):
      drupal_goto('csa/box/unpublish');

    case t(CSA_BOX_ACTION_TRANSCODE):
      drupal_goto('csa/box/transcode/' . $values['type']);

    case t(CSA_BOX_ACTION_METADATA):
      drupal_goto('csa/box/metadata/' . $values['type']);

    case t(CSA_BOX_ACTION_ACCESS_RULES):
      drupal_goto('csa/box/access/' . $values['type']);

    case t(CSA_BOX_ACTION_GENERATE_STILLS):
      drupal_goto('csa/box/generatestill/' . $values['type']);

    case t(CSA_BOX_ACTION_ADD_TO_COLLECTION):
      drupal_goto('csa/box/addtocollection/' . $values['type']);

    case t(CSA_BOX_ACTION_REMOVE_FROM_COLLECTION):
      drupal_goto('csa/box/removefromcollection/' . $values['type']);

    case t(CSA_BOX_ACTION_BROWSE_SELECTION):
      // Get the first asset_id.
      $asset_ids = _csa_box_get_asset_listing_selection(array('offset' => 0, 'limit' => 1));

      assert(!empty($asset_ids));

      drupal_goto('csa/' . arg(1) . '/asset/' . $asset_ids[0] . '/0/' . CSA_MODE_SELECTION);

    default:
      // If we get here then something went wrong.
      drupal_set_message(t("'Unknown action '@action'", array('@action' => $values['action'])), 'error');
      _csa_log_critical('Submit form of @box was not valid!', array('@box' => $values['type']));
      break;
  }
}

/**
 * Shows the search form for search page.
 */
function _csa_box_search_form($form_state, $options = array()) {

  // JS.
  drupal_add_js(drupal_get_path('module', 'csa_box') . '/csa_box.js');

  $form = array();

  // CSS.
  drupal_add_css(drupal_get_path('module', 'csa') . '/csa.css', 'module');
  drupal_add_css(drupal_get_path('module', 'csa_box') . '/csa_box.css', 'module');

  // Get the values.
  $values = empty($_SESSION['csa_forms']['search']['values']) ? array() : $_SESSION['csa_forms']['search']['values'];

  // Get the drop down listing of metadata properties.
  $metadata = _csa_asset_metadata_list_misc();
  $metadata_options = $metadata['metadata_options'];

  // Set default.
  $values += array(
    'search_operator' => 'and',
    'cql' => '',
    'search_collection' => 0,
  );

  // Get listing collections.
  $collections = _csa_collection_search(array('for_select' => TRUE));
  if (!empty($collections)) {
    $collections = array_merge(array(0 => ''), $collections);
  }

  $form['filter'] = array(
    '#type' => 'fieldset',
    '#title' => 'Filter',
    '#weight' => -5,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['filter']['search_operator'] = array(
    '#type' => 'select',
    '#title' => t('Search criteria operator'),
    '#description' => t("Filter on all search criteria ('AND') or a least one criterium ('OR')."),
    '#options' => array('or' => t('OR'), 'and' => t('AND')),
    '#default_value' => $values['search_operator'],
    '#weight' => -5,
  );

  if (!empty($collections)) {
    $form['filter']['search_collection'] = array(
      '#type' => 'fieldset',
      '#title' => 'Search within collection',
      '#weight' => 0,
      '#collapsible' => TRUE,
      '#collapsed' => empty($values['search_collection']),
    );

    $form['filter']['search_collection']['search_collection'] = array(
      '#type' => 'select',
      '#title' => t('Collection'),
      '#description' => t('Search within the selected collection.'),
      '#options' => $collections,
      '#default_value' => $values['search_collection'],
      '#weight' => -4,
    );
  }
  else {
    $form['search_collection'] = array('#type' => 'hidden', '#value' => 0);
  }

  $form['filter']['search_published'] = array(
    '#type' => 'fieldset',
    '#title' => 'Publish status',
    '#weight' => 1,
    '#collapsible' => TRUE,
    '#collapsed' => empty($values['search_published']),
  );

  $form['filter']['search_published']['search_published'] = array(
    '#type' => 'select',
    '#title' => t('Publish status'),
    '#description' => t('Search on the published status of the media.'),
    '#options' => array(0 => t('All'), 1 => t('Only published'), 2 => t('Only not published')),
    '#default_value' => $values['search_published'],
    '#weight' => -3,
  );

  $form['filter']['cql'] = array(
    '#type' => 'fieldset',
    '#title' => 'CQL',
    '#weight' => 2,
    '#collapsible' => TRUE,
    '#collapsed' => empty($values['cql']),
  );

  $form['filter']['cql']['cql'] = array(
    '#type' => 'textfield',
    '#title' => t('CQL string'),
    '#description' => t("Specify CQL search string to search."),
    '#weight' => -3,
    '#default_value' => $values['cql'],
    '#attributes' => array('onkeypress' => "csa_submit_search(event);"),
  );

  $form['filter']['search'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SEARCH_OP_APPLY_AND_SEARCH),
    '#weight' => 20,
  );

  $form['filter']['reset_prop'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SEARCH_OP_RESET_METADATA),
    '#weight' => 21,
  );

  // Metadata search.
  $form['filter']['metadata'] = array(
    '#type' => 'fieldset',
    '#title' => 'Metadata search',
    '#weight' => 5,
    '#collapsible' => TRUE,
    '#collapsed' => empty($_SESSION['csa_forms'][$values['type']]['search_metadata']),
  );

  $form['filter']['metadata']['search_metadata_prop'] = array(
    '#type' => 'select',
    '#title' => t('Metadata property'),
    '#options' => $metadata_options,
    '#attributes' => array('class' => 'csa-metadata-filter-add'),
    '#size' => 1,
    '#weight' => -5,
  );

  $form['filter']['metadata']['add_prop'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SEARCH_OP_ADD_METADATA),
    '#weight' => 10,
  );

  if (!empty($_SESSION['csa_forms'][$values['type']]['search_metadata'])) {
    // Search options metadata.
    foreach ($_SESSION['csa_forms'][$values['type']]['search_metadata'] as $prop_id => $items) {
      foreach ($items as $key_item => $item) {
        if (empty($metadata['metadata_propid_to_name'][$prop_id])) {
          continue;
        }

        $group_title = '';
        foreach ($metadata['metadata_options'] as $key_group_title => $properties) {
          if (isset($properties[$prop_id])) {
            $group_title = $key_group_title;
            break;
          }
        }

        $item += array(
          'value' => empty($_SESSION['csa_forms'][$values['type']]['values']['prop_id_' . $prop_id . '_' . $key_item]) ? '' : $_SESSION['csa_forms'][$values['type']]['values']['prop_id_' . $prop_id . '_' . $key_item],
          'value_till' => empty($_SESSION['csa_forms'][$values['type']]['values']['prop_id_' . $prop_id . '_' . $key_item . '_till']) ? '' : $_SESSION['csa_forms'][$values['type']]['values']['prop_id_' . $prop_id . '_' . $key_item . '_till'],
          'name' => $metadata['metadata_propid_to_name'][$prop_id],
          'type' => $metadata['metadata_propid_to_type'][$prop_id],
          'title' => $metadata['metadata_propid_to_title'][$prop_id],
          'group_title' => $group_title,
        );

        switch (drupal_strtoupper($item['type'])) {
          case 'CHAR':
          case 'INT':
            $form['filter']['metadata']['prop_id_' . $prop_id . '_' . $key_item] = array(
              '#type' => 'textfield',
              '#title' => t($item['title']) . (empty($item['group_title']) ? '' : ' (' . $item['group_title'] . ')'),
              '#weight' => 20,
              '#default_value' => $item['value'],
            );
            break;

          case 'DATETIME':
            $form['filter']['metadata']['date_' . $prop_id . '_' . $key_item] = array(
              '#value' => t('From (YYYY[-MM-DD HH:MM:SS]) - till (YYYY[-MM-DD HH:MM:SS]). Till is optional.'),
              '#weight' => 20,
              '#prefix' => '<div><label for="date_' . $prop_id . '_' . $key_item . '">' . check_plain(t($item['title']) . (empty($item['group_title']) ? '' : ' (' . $item['group_title'] . ')')) . ':</label><br />',
              '#suffix' => '</div><br /><br />',
            );

            $form['filter']['metadata']['date_' . $prop_id . '_' . $key_item]['prop_id_' . $prop_id . '_' . $key_item] = array(
              '#type' => 'textfield',
              '#weight' => 22,
              '#default_value' => $item['value'],
              '#attributes' => array('class' => 'csa-metadata-filter-datetime-lft'),
            );

            $form['filter']['metadata']['date_' . $prop_id . '_' . $key_item]['till'] = array(
              '#weight' => 23,
              '#value' => ' ',
              '#prefix' => '<div class="csa-metadata-filter-till"><label for="date_' . $prop_id . '_' . $key_item . '_till">till</label><br />',
              '#suffix' => '</div>',
            );

            $form['filter']['metadata']['date_' . $prop_id . '_' . $key_item]['prop_id_' . $prop_id . '_' . $key_item . '_till'] = array(
              '#type' => 'textfield',
              '#weight' => 24,
              '#default_value' => $item['value_till'],
              '#attributes' => array('class' => 'csa-metadata-filter-datetime-lft'),
            );

            break;

          default:
            assert(0);
            break;
        }
      }
    }
  }

  $form['#validate'][] = '_csa_box_search_form_validate';
  $form['#submit'][] = '_csa_box_search_form_submit';

  return $form;
}

/**
 * Validate CQL input.
 */
function _csa_box_search_form_validate($form, &$form_state) {
}

/**
 * Submit the CQL.
 */
function _csa_box_search_form_submit($form, &$form_state) {
  // Get the values;
  $values = $form_state['values'];

  // Get pressed button.
  $op = $values['op'];

  // Depending of the type, we need another confirm or we'll just start.
  switch ($op) {
    case t(CSA_SEARCH_OP_APPLY_AND_SEARCH):
      $_SESSION['csa_forms'][$values['type']]['values'] = $form_state['values'];

      // Unselect any selected items (search results will change).
      _csa_box_item_unset_selected(CSA_BOX_TYPE_SEARCH, array(), TRUE);

      // Do search.
      $form_state['storage']['do_search'] = TRUE;
      break;

    case t(CSA_SEARCH_OP_ADD_METADATA):
      unset($form_state['storage']['do_search']);

      // Unselect any selected items (search results will change).
      _csa_box_item_unset_selected(CSA_BOX_TYPE_SEARCH, array(), TRUE);

      // Need to store possible changes.
      $_SESSION['csa_forms'][$values['type']]['values'] = $form_state['values'];

      // Adding new item.
      $_SESSION['csa_forms'][$values['type']]['search_metadata'][$values['search_metadata_prop']][] = array();
      break;

    case t(CSA_SEARCH_OP_RESET_METADATA):
      unset($form_state['storage']['do_search']);

      // Unselect any selected items (search results will change).
      _csa_box_item_unset_selected(CSA_BOX_TYPE_SEARCH, array(), TRUE);

      // Clean session.
      _csa_box_search_clean_session();
      break;

    default:
      $_SESSION['csa_forms'][$values['type']]['values'] = $form_state['values'];
      unset($form_state['storage']['do_search']);
      break;
  }
}

/**
 * Create a clean state of search form.
 */
function _csa_box_search_clean_session() {
  // Remove metadata search params.
  unset($_SESSION['csa_forms'][CSA_BOX_TYPE_SEARCH]['values']);
  unset($_SESSION['csa_forms'][CSA_BOX_TYPE_SEARCH]['search_metadata']);
}

/**
 * Publish the selected items.
 */
function _csa_box_publish() {
  // Call batch.
  _csa_box_batch_publish();
}

/**
 * Unpublish the selected items.
 */
function _csa_box_unpublish() {
  // Call batch
  _csa_box_batch_unpublish();
}

/**
 * Delete the selected items.
 *
 * @param string $type
 */
function _csa_box_delete_form($form_state, $type) {
  $form['type'] = array('#type' => 'hidden', '#value' => $type);

  // Get item count.
  $item_count = _csa_box_item_number_of_selected($type);
  assert($item_count);

  return confirm_form($form,
                      format_plural(
                        $item_count,
                        'You have selected one asset for deletion. Are you sure?',
                        'You have selected @count assets for deletion. Are you sure?',
                        array('@count' => $item_count)
                      ),
                      'csa/' . $type,
                      t('This action cannot be undone.'),
                      t('Delete selected assets'),
                      t('Cancel')
  );
}

/**
 * Delete selected items.
 */
function _csa_box_delete_form_submit($form, &$form_state) {

  switch ($form_state['values']['op']) {
    case t('Delete selected assets'):
      drupal_goto('/csa/box/delete/' . $form_state['values']['type']);

    default:
    case t('Cancel'):
      drupal_set_message('You have canceled the deletion.');
      drupal_goto('/csa/' . $form_state['values']['type']);
  }
}

/**
 * User has confirmed, delete the selection.
 */
function _csa_box_delete($type) {
  // Make sure its valid.
  if (!_csa_box_check_type($type, FALSE)) {
    drupal_goto('/csa');
  }

  // Run batch.
  _csa_box_batch_delete($type);
}

/**
 * Publish the selected items. Will show form for transcode options first.
 *
 * @param string $type
 */
function _csa_box_transcode_form($form_state, $type) {

  // Will throw if not correct.
  _csa_box_check_type($type);

  // Get the basic form.
  return _csa_asset_mediafile_transcode_select_form($form_state, $type);
}

/**
 * Validate the form.
 *
 * @param array $form
 * @param array $form_state
 */
function _csa_box_transcode_form_validate($form, &$form_state) {
  switch ($form_state['values']['op']) {
    case t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_CANCEL):
      form_set_error(NULL, '', TRUE); // Prevent problems with missing fields.
      drupal_get_messages('error', TRUE);

      // Go back.
      drupal_goto('csa/' . $form_state['values']['type']);
      break;
  }
}

/**
 * Submit the form.
 *
 * @param array $form
 * @param array $form_state
 */
function _csa_box_transcode_form_submit($form, $form_state) {

  switch ($form_state['values']['op']) {
    case CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_START_TRANSCODINGS:
      _csa_box_batch_transcode($form_state['values']['type'], $form_state['values']['profile_ids'], $form_state['values']['transcode_replace']);
      break;
  }
}

/**
 * Publish the selected items. Will show form for transcode options first.
 *
 * @param string $type
 */
function _csa_box_access_form($form_state, $type) {
  // Will throw if not correct.
  _csa_box_check_type($type);

  // Get the basic form.
  $form = _csa_asset_mediafile_access_select_form($form_state, $type);

  $form['submit']['start'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_START_ACCESS),
  );

  $form['submit']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_CANCEL_ACCESS),
  );

  $form['#validate'][] = '_csa_box_access_form_validate';
  $form['#submit'][] = '_csa_box_access_form_submit';
  return $form;
}

/**
 * Validate function for access form.
 */
function _csa_box_access_form_validate($form, $form_state) {
  // Get the values;
  $values = $form_state['values'];
  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_START_ACCESS):
      if (empty($form_state['storage']['access']['domains']) && empty($form_state['storage']['access']['realms'])) {
        form_set_error('selected_access_rules', t('Add one or more access rules to start the batch.'));
      }
      break;
  }
}

/**
 * Submit function for access form.
 */
function _csa_box_access_form_submit($form, &$form_state) {
  // Get the values;
  $values = $form_state['values'];

  switch ($values['op']) {
    case t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_START_ACCESS):
      _csa_box_batch_access_rules($form_state['values']['type'], $form_state['storage']['access']);
      break;

    case t(CSA_SUBMIT_TEXT_ASSET_MEDIAFILE_CANCEL_ACCESS):
      drupal_goto('csa/' . $form_state['values']['type']);
  }
}

/**
 * Change/add metadata to the selected items. Will show form for metadata.
 * options first.
 *
 * @param string $type
 */
function _csa_box_metadata_form(&$form_state, $type) {
  return _csa_asset_metadata_form($form_state, NULL, -1, CSA_MODE_QUERY, array('type' => $type, 'skip_required_fields' => TRUE));
}

/**
 * Validate input.
 *
 * @param array $form
 * @param array $form_state
 */
function _csa_box_metadata_form_validate($form, &$form_state) {

  $values = $form_state['values'];

  switch ($values['op']) {
    case t(CSA_ASSET_METADATA_CANCEL):
      drupal_goto('csa/' . $values['type']); // go back.

    case t(CSA_ASSET_METADATA_SAVE_METADATA):
      // Check if we entered metadata.
      $empty_submit = TRUE;

      foreach ($values as $name => $value) {
        // Any empty values we skip.
        if (trim($value) == '') {
          continue;
        }

        if (substr($name, 0, drupal_strlen(CSA_ASSET_METADATA_PREFIX)) ==  CSA_ASSET_METADATA_PREFIX) {
          $empty_submit = FALSE;
          break;
        }
      }

      if ($empty_submit) {
        form_set_error('foo', t("You haven't entered any metadata, press 'cancel' if you wish to stop."));
      }
  }

  // Call parent.
  _csa_asset_metadata_form_validate($form, $form_state);
}

/**
 * Submit form.
 *
 * @param array $form
 * @param array $form_state
 */
function _csa_box_metadata_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  switch ($values['op']) {

    case t(CSA_ASSET_METADATA_ADD_FIELD_OPTION):
      $form_state['storage']['metadata_group_add'] = array_merge(isset($form_state['storage']['metadata_group_add']) ? $form_state['storage']['metadata_group_add'] : array(), $values['metadata_group_add']);

      // Prevent losing our posted data.
      $form_state['rebuild'] = TRUE;
      break;

    case t(CSA_ASSET_METADATA_SAVE_METADATA):
      $metadata = array();

      foreach ($values as $name => $value) {
        // Any empty values we skip.
        if (trim($value) == '') {
          continue;
        }

        // Name is in format of [CSA_ASSET_METADATA_PREFIX] [name] [_#]
        if (substr($name, 0, drupal_strlen(CSA_ASSET_METADATA_PREFIX)) ==  CSA_ASSET_METADATA_PREFIX) {
          $name = substr($name, drupal_strlen(CSA_ASSET_METADATA_PREFIX));

          $name = explode('_', $name);
          $key = array_pop($name);// remove key.

          $metadata[implode('_', $name)][] = trim($value);
        }
      }

      // we are done, so lets start the batch, will redirect when done.
      _csa_box_batch_common_metadata($values['type'], $metadata);
      break;
  }
}

/**
 * Show asset detail page.
 *
 * @param string $asset_id
 */
function _csa_box_asset_detail($asset_id, $offset, $mode) {
  $options['link_metadata_edit'] = 'csa/' . arg(1) . '/asset/!asset_id/!offset/!mode/edit';
  $options['link_mediafile_delete'] = 'csa/mediafile/!mediafile_id/delete';
  $options['link_mediafile_access'] = 'csa/mediafile/!mediafile_id/access';

  // Precache globals.
  theme('csa_box_previous_next_asset', 'csa/' . arg(1) . '/asset/!asset_id/!offset/!mode', $offset, $mode);

  return _csa_asset_detail($asset_id, $offset, $mode, $options);
}

/**
 * Show asset detail page (in/outbox).
 *
 * @param string $asset_id
 */
function _csa_box_asset_metadata_edit($asset_id, $offset, $mode) {
  // Get the asset.
  $asset = _csa_asset_get($asset_id);

  // Get the title.
  $title = _csa_asset_get_title($asset);

  $options['link_metadata_edit'] = 'csa/' . arg(1) . '/asset/!asset_id/!offset/!mode/edit';
  return _csa_asset_metadata_edit($asset_id, $offset, $mode, $options);
}

/**
 * Move batch into collection. Show a select collection form.
 *
 * @param string $type
 */
function _csa_box_move_to_collection_form($form_state, $type) {

  // Will throw if not correct.
  _csa_box_check_type($type);

  // Get the basic form.
  return _csa_collection_select_form($form_state, $type);
}

/**
 * Validate the form.
 *
 * @param array $form
 * @param array $form_state
 */
function _csa_box_move_to_collection_form_validate($form, &$form_state) {
  switch ($form_state['values']['op']) {
    case t(CSA_SUBMIT_TEXT_COLLECTION_SELECT_CANCEL):
      form_set_error(NULL, '', TRUE); // Prevent problems with missing fields.
      drupal_get_messages('error', TRUE);

      // Go back.
      drupal_goto('csa/' . $form_state['values']['type']);
      break;
  }
}

/**
 * Submit the form.
 *
 * @param array $form
 * @param array $form_state
 */
function _csa_box_move_to_collection_form_submit($form, $form_state) {

  switch ($form_state['values']['op']) {
    case CSA_SUBMIT_TEXT_COLLECTION_SELECT:
      _csa_box_batch_move_into_collection($form_state['values']['type'], $form_state['values']['coll_id']);
      break;
  }
}

/**
 * Move batch into collection. Show a select collection form.
 *
 * @param string $type
 */
function _csa_box_remove_from_collection_form($form_state, $type) {

  // Will throw if not correct.
  _csa_box_check_type($type);

  // Get the basic form.
  return _csa_collection_select_form($form_state, $type);
}

/**
 * Validate the form.
 *
 * @param array $form
 * @param array $form_state
 */
function _csa_box_remove_from_collection_form_validate($form, &$form_state) {
  switch ($form_state['values']['op']) {
    case t(CSA_SUBMIT_TEXT_COLLECTION_SELECT_CANCEL):
      form_set_error(NULL, '', TRUE); // Prevent problems with missing fields.
      drupal_get_messages('error', TRUE);

      // Go back.
      drupal_goto('csa/' . $form_state['values']['type']);
      break;
  }
}

/**
 * Submit the form.
 *
 * @param array $form
 * @param array $form_state
 */
function _csa_box_remove_from_collection_form_submit($form, $form_state) {

  switch ($form_state['values']['op']) {
    case CSA_SUBMIT_TEXT_COLLECTION_SELECT:
      _csa_box_batch_remove_from_collection($form_state['values']['type'], $form_state['values']['coll_id']);
      break;
  }
}

/**
 * Show still generate form.
 *
 * @param array $form_state
 * @param string $type
 */
function _csa_box_generatestill_form($form_state, $type) {
  // Will throw if not correct.
  _csa_box_check_type($type);

  // Get the basic form.
  return _csa_still_create_form($form_state, array('hidden' => array('type' => $type)));
}

/**
 * Validate the form.
 *
 * @param array $form
 * @param array $form_state
 */
function _csa_box_generatestill_form_validate($form, &$form_state) {
  switch ($form_state['values']['op']) {
    case t(CSA_SUBMIT_TEXT_GENERATE_STILL_CANCEL):
      form_set_error(NULL, '', TRUE); // Prevent problems with missing fields.
      drupal_get_messages('error', TRUE);

      // Go back.
      drupal_goto('csa/' . $form_state['values']['type']);
      break;
  }
}

/**
 * Submit the form.
 *
 * @param array $form
 * @param array $form_state
 */
function _csa_box_generatestill_form_submit($form, $form_state) {

  switch ($form_state['values']['op']) {
    case CSA_SUBMIT_TEXT_GENERATE_STILL:

      $params = array(
        'still_type',
        'still_per_mediafile',
        'still_every_second',
        'start_time',
        'end_time',
        'size',
        'h_padding',
        'v_padding',
      );

      // Copy the settings.
      $still_options = array();
      foreach ($params as $param) {
        assert(isset($form_state['values'][$param]));
        $still_options[$param] = $form_state['values'][$param];
      }

      _csa_box_batch_generate_still($form_state['values']['type'], $still_options);
      break;
  }
}

/**
 * returns the text about selection.
 *
 * @param string $type
 */
function _csa_box_get_selected_text($type) {
  // Output the quota.
  _csa_json_output(_csa_box_item_get_selected_text($type));
  exit();
}
