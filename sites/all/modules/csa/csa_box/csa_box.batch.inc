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
 * The batch contains a selection of assets and executes a action on them.
 */

/**
 * If during the batch the asset is gone for some reason, we ignore it. Else
 * we stop the batch and keep on trying to run batch on IDs that are no longer
 * in our database.
 *
 * @param unknown_type $asset_id
 */
function _csa_box_batch_get_asset($asset_id, &$context = NULL) {
  $asset = FALSE;

  try {
    $asset = _csa_asset_get($asset_id, array('show_stills' => FALSE, 'fatal' => TRUE));
    $asset = reset($asset['items']['item']);
  }
  catch (Exception $e) {
    // If we didn't find the asset, then its deleted right?
    if ($e->getCode() != mediamosa_error::ERRORCODE_ASSET_NOT_FOUND) {
      throw $e;
    }

    if (!empty($context)) {
      $context['message'] = t('Notice: asset with ID @asset_id was not found during batch, ignoring and continuing to next asset.', array('@asset_id' => $asset_id));

      // Make sure we dont overflow our memory usage with our logging.
      if (count($context['results']['log']) > 100) {
        array_shift($context['results']['log']); // remove the first one.
      }

      $context['results']['log'][] = $context['message'];

      // Log in watchdog.
      _csa_log_warning('Asset with ID @asset_id was not found during batch, ignoring and continuing to next asset.', array('@asset_id' => $asset_id));
    }

  }

  return $asset;
}

/**
 * Activates the batch for you.
 *
 * @param string $function_name
 *   The function to call with each item.
 * @param string $type
 *   unpublished/published.
 * @param array $batch
 *  Preset batch array.
 */
function _csa_box_batch($redirect, $options, $batch = array()) {

  assert(isset($options['function_name']));
  assert(isset($options['success_message']));
  assert(isset($options['failure_message']));
  assert(isset($options['type']));

  // Hack the progress message.
  $options['progress_message'] = $batch['progress_message'];
  unset($batch['progress_message']);

  $batch += array(
    'operations' => array(
      array('_csa_box_batch_run', array($options)),
    ),
    'finished' => '_csa_box_batch_finished',

    'title' => t('Need title here'),
    'init_message' => t('Starting batch.'),
    'progress_message' => '<!-- ',
    'error_message' => t('Batch has encountered an error.'),
  );

  batch_set($batch);

  // Start it.
  batch_process($redirect);
}

/**
 * Does a single run in the batch.
 *
 * @param string $function_name
 *   The function to call with each item.
 * @param string $type
 *   unpublished/published.
 * @param array $context
 *  Shared context array.
 */
function _csa_box_batch_run($options, &$context) {

  // Set count to at least 1.
  $options['count'] = empty($options['count']) ? 1 : $options['count'];

  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['results']['max']  = $context['sandbox']['max'] = _csa_box_item_number_of_selected($options['type']);
    $context['results']['count'] = 0; // Total items done.
    $context['results']['options'] = $options;
    $context['results']['success'] = TRUE; // All went well as default.
    $context['results']['log'] = array();
  }

  // Make sure its hidden.
  $context['message'] = '';

  // All done ?
  if (empty($context['sandbox']['max'])) {
    return;
  }

  // Set progress message.
  $values = array(
    '@remaining'  => $context['results']['max'] - (min($context['results']['max'], $context['sandbox']['progress']  + $options['count'])),
    '@total'      => $context['results']['max'],
    '@current'    => min($context['results']['max'], $context['results']['count'] + $options['count']),
  );
  $message = strtr($options['progress_message'], $values);

  // Get the next item(s).
  $asset_id = _csa_box_item_next($options['type'], $options['count']);

  // All done ?
  if (empty($asset_id)) {
    return;
  }

  // Call the function.
  try {
    // Set message now in case it fails.
    $context['message'] = '--> ' . (empty($context['message']) ? $message : $message . '<br />' . $context['message']);

    $options['function_name']($context, $options['type'], $asset_id);

    if (substr($context['message'], 0, 3) != '-->') {
      $context['message'] = '--> ' . (empty($context['message']) ? $message : $message . '<br />' . $context['message']);
    }
  }
  catch (Exception $e) {
    // Something went wrong.

    // Unselect the problem, because the user might not be able to!
    _csa_box_item_unset_selected($options['type'], (is_array($asset_id) ? $asset_id : array($asset_id)));

    $context['results']['success'] = FALSE;
    $context['results']['exceptions'][] = array('code' => $e->getCode(), 'message' => $e->getMessage());
    return;
  }

  // Ok done successful action, now remove it.
  _csa_box_item_unset_selected($options['type'], (is_array($asset_id) ? $asset_id : array($asset_id)));

  // Next progress.
  $context['sandbox']['progress'] += (is_array($asset_id) ? count($asset_id) : 1);

  if ($context['sandbox']['progress'] < $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Called when batch was finished.
 *
 * @param boolean $success
 * @param array $results
 * @param array $operations
 */
function _csa_box_batch_finished($success, $results, $operations) {

  // $success only says the batch code didnt have problems, not my code.
  $success = $success ? $results['success'] : $success;

  if ($success) {
    $success_message = $results['options']['success_message'];
    $success_message = format_plural($results['count'], $success_message['singular'], $success_message['plural']);
    $success_message = t('Batch completed sucessfully. !success_message', array('!success_message' => $success_message));
    drupal_set_message($success_message);
    _csa_log_info($success_message);
  }
  else {
    $done = $results['count'];
    $total = $results['max'];

    $failure_message = t($results['options']['failure_message'], array('@failed' => $total - $done, '@total' => $total));
    $failure_message = t('Batch halted; failed to complete the batch. !failure_message', array('!failure_message' => $failure_message));

    if (!empty($results['exceptions'])) {
      $exceptions = array('<ul>');
      foreach ($results['exceptions'] as $exception) {
        $exceptions[] = '<li>' . $exception['message'] . '</li>';
      }
      $exceptions[] = '</ul>';

      $failure_message .= '<p>' . t('Error message(s) during batch: !messages', array('!messages' => implode('', $exceptions))) .  '</p>';
    }

    drupal_set_message($failure_message, 'error');
    _csa_log_warning($failure_message);
  }
}

/**
 * Batch for deleting assets.
 *
 * @param string $type
 *   unpublished/published.
 */
function _csa_box_batch_delete($type) {

  _csa_box_batch(
    url('csa/' . $type),
    array(
      'function_name' => '_csa_box_batch_delete_asset',
      'type' => $type,
      'success_message' => array(
        'singular' => 'Deleted 1 asset.',
        'plural' => 'Deleted @count assets.',
      ),
      'failure_message' => 'Failed to delete @failed assets of the @total assets.',
    ),
    array(
      'title' => t('Deleting assets'),
      'init_message' => t('Starting batch to delete selected assets.'),
      'progress_message' => t('Deleting @current asset out of @total  (@remaining remaining).'),
      'error_message' => t('Delete asset batch has encountered an error.'),
    )
  );
}

/**
 * Function called within the batch.
 *
 * @param array $context
 * @param string $type
 * @param string $mediafile_id
 */
function _csa_box_batch_delete_asset(&$context, $type, $asset_id) {

  // Get asset.
  $asset = _csa_box_batch_get_asset($asset_id);

  if ($asset) {
    // Get the owner of the asset.
    $user_id = $asset['owner_id'];

    // Delete the asset.
    _csa_asset_delete($asset_id, $user_id, array('fatal' => TRUE, 'delete' => 'cascade'));
  }

  // Add the asset ID for result.
  $context['results']['count']++;

  // Set the title.
  $context['message'] = t("Deleted asset '@title'", array('@title' => _csa_asset_get_title($asset)));
}

/**
 * Batch for publishing assets.
 */
function _csa_box_batch_publish() {
  global $user;

  // Make sure the access rules are set.
  _csa_prefs_access_precheck_rules();

  _csa_box_batch(
    url('csa/unpublished'),
    array(
      'function_name' => '_csa_box_batch_publish_asset',
      'type' => CSA_BOX_TYPE_UNPUBLISHED,
      'success_message' => array(
        'singular' => 'Published 1 asset.',
        'plural' => 'Published @count assets.',
      ),
      'failure_message' => 'Failed to publish @failed assets of the @total assets.',
      // Publish settings.
      'aut_app' => array_keys($user->csa_access_rules['applications']),
      'aut_domain' => $user->csa_access_rules['domains'],
      'aut_realm' => $user->csa_access_rules['realms'],
    ),
    array(
      'title' => t('Publishing assets'),
      'init_message' => t('Starting batch to publish selected assets.'),
      'progress_message' => t('Publishing @current asset out of @total  (@remaining remaining).'),
      'error_message' => t('Publishing assets batch has encountered an error.'),
    )
  );
}

/**
 * Publish asset.
 *
 * @param array $context
 * @param string $mediafile_id
 */
function _csa_box_batch_publish_asset(&$context, $type, $asset_id) {

  // Get asset.
  $asset = _csa_box_batch_get_asset($asset_id, $context);

  if ($asset) {
    // Get the owner of the asset.
    $user_id = $asset['owner_id'];

    // @todo: Before we publish, check if the required metadata is set.


    // All ok, now publish.
    $options = array(
      'aut_app' => $context['results']['options']['aut_app'],
      'aut_domain' => $context['results']['options']['aut_domain'],
      'aut_realm' => $context['results']['options']['aut_realm'],
      'fatal' => TRUE,
    );

    // Now loop through the mediafiles (if any) and set ACL.
    foreach ($asset['mediafiles']['mediafile'] as $mediafile) {
      _csa_asset_mediafile_set_acl($mediafile['mediafile_id'], $mediafile['owner_id'], $options);
    }

    // Set the title.
    $context['message'] = t("Published asset '@title'", array('@title' => _csa_asset_get_title($asset)));
  }

  // Add the asset ID for result.
  $context['results']['count']++;
}

/**
 * Batch for publishing assets.
 */
function _csa_box_batch_unpublish() {
  global $user;

  _csa_box_batch(
    url('csa/published'),
    array(
      'function_name' => '_csa_box_batch_unpublish_asset',
      'type' => CSA_BOX_TYPE_PUBLISHED,
      'success_message' => array(
        'singular' => 'Unpublished 1 asset.',
        'plural' => 'Unpublished @count assets.',
      ),
      'failure_message' => 'Failed to unpublish @failed assets of the @total assets.',
      // Publish settings.
      'aut_app' => array_keys($user->csa_access_rules['applications']),
      'aut_domain' => $user->csa_access_rules['domains'],
      'aut_realm' => $user->csa_access_rules['realms'],
    ),
    array(
      'title' => t('Unpublishing assets'),
      'init_message' => t('Starting batch to unpublish selected assets.'),
      'progress_message' => t('Unpublishing @current asset out of @total  (@remaining remaining).'),
      'error_message' => t('Unpublishing assets batch has encountered an error.'),
    )
  );
}

/**
 * Unpublish asset.
 *
 * @param array $context
 * @param string $mediafile_id
 */
function _csa_box_batch_unpublish_asset(&$context, $type, $asset_id) {

  // Get asset.
  $asset = _csa_box_batch_get_asset($asset_id, $context);

  if ($asset) {
    // Get the owner of the asset.
    $user_id = $asset['owner_id'];

    // Remove all ACL.
    _csa_asset_delete_acl($asset_id, $user_id);

    // Set the title.
    $context['message'] = t("Unpublished asset '@title'", array('@title' => _csa_asset_get_title($asset)));
  }

  // Add the asset ID for result.
  $context['results']['count']++;
}

/**
 * Batch for publishing assets.
 */
function _csa_box_batch_common_metadata($type, $metadata) {
  global $user;

  _csa_box_batch(
    url('csa/' . $type),
    array(
      'function_name' => '_csa_box_batch_common_metadata_asset',
      'type' => $type,
      'success_message' => array(
        'singular' => 'Added metadata on 1 asset.',
        'plural' => 'Added metadata on @count assets.',
      ),
      'failure_message' => 'Failed to set metadata on @failed assets of the @total assets.',
      'metadata' => $metadata,
    ),
    array(
      'title' => t('Adding metadata on assets'),
      'init_message' => t('Starting batch for adding metadata to selected assets.'),
      'progress_message' => t('Adding metadata @current asset out of @total  (@remaining remaining).'),
      'error_message' => t('Adding metadata batch has encountered an error.'),
    )
  );
}

/**
 * Common metadata.
 *
 * @param array $context
 * @param string $type
 * @param string $asset_id
 */
function _csa_box_batch_common_metadata_asset(&$context, $type, $asset_id) {
  // Get asset.
  $asset = _csa_box_batch_get_asset($asset_id, $context);

  if ($asset) {
    // Get the owner of the asset.
    $user_id = $asset['owner_id'];

    // Save metadata.
    _csa_asset_metadata_save($asset_id, $context['results']['options']['metadata'], array('action' => 'update', 'set_message' => FALSE, 'fatal' => TRUE));

    // Set the title.
    $context['message'] = t("Added metadata to asset '@title'", array('@title' => _csa_asset_get_title($asset)));
  }

  // Add the asset ID for result.
  $context['results']['count']++;
}

/**
 * Batch for transcoding.
 */
function _csa_box_batch_transcode($type, $profile_ids, $transcode_replace) {
  global $user;

  _csa_box_batch(
    url('csa/' . $type),
    array(
      'function_name' => '_csa_box_batch_transcode_asset',
      'type' => $type,
      'success_message' => array(
        'singular' => 'Started transcoding on 1 asset.',
        'plural' => 'Started transcoding on @count assets.',
      ),
      'failure_message' => 'Failed to start transcoding on @failed assets of the @total assets.',
      'profile_ids' => $profile_ids,
      'transcode_replace' => $transcode_replace,
    ),
    array(
      'title' => t('Starting transcoding'),
      'init_message' => t('Starting batch for transcoding selected assets.'),
      'progress_message' => t('Starting transcoding job, @current asset out of @total  (@remaining remaining).'),
      'error_message' => t('The transcode batch has encountered an error.'),
    )
  );
}

/**
 * Transcode asset(s).
 *
 * @param array $context
 * @param string $type
 *   unpublished/published.
 * @param string $mediafile_id
 *   Any mediafile, will search for original.
 */
function _csa_box_batch_transcode_asset(&$context, $type, $asset_id) {

  // Get asset.
  $asset = _csa_box_batch_get_asset($asset_id, $context);

  if ($asset) {
    // Find the original.
    if (!empty($asset['mediafiles']['mediafile'])) {
      foreach ($asset['mediafiles']['mediafile'] as $key => $mediafile) {
        unset($asset['mediafiles']['mediafile'][$key]['@attributes']);

        if (drupal_strtoupper($mediafile['is_original_file']) == 'TRUE') {
          // Set the original mediafile id for this asset.
          $asset['mediafile_original'] = $mediafile;
        }
      }
    }

    if (!isset($asset['mediafile_original'])) {
      // If we can't find the original, then throw exception.
      throw new Exception(t('Asset with ID @asset_id (@title) does not have an original mediafile, can not transcode with out original.', array('@asset_id' => $asset_id, '@title' => _csa_asset_get_title($asset))));
    }

    // Ok got original, now transcode.

    // Get the owner of the mediafile.
    $user_id = $asset['mediafile_original']['owner_id'];

    // Now loop through the transcode ids.
    foreach ($context['results']['options']['profile_ids'] as $profile_id) {

      $options['completed_transcoding_url'] = '';

      // If we selected replace/skip, turn on the 'completed_transcoding_url' function.
      $mediafile_ids_to_remove = array();

      switch ($context['results']['options']['transcode_replace']) {
        case CSA_TRANSCODE_REPLACE_OPTION_REPLACE:
        case CSA_TRANSCODE_REPLACE_OPTION_SKIP:
          // Find the other mediafile with same transcode profile to delete.
          foreach ($asset['mediafiles']['mediafile'] as $key => $mediafile) {
            if ($mediafile['transcode_profile_id'] == $profile_id && $mediafile['is_still'] == 'FALSE') {
              // Same, ok delete when transcode is done.
              $mediafile_ids_to_remove[] = $mediafile['mediafile_id'];
            }
          }

          // Any to remove?
          if (!empty($mediafile_ids_to_remove)) {
            // Because the trigger is not with cookie, we need to make sure we
            // setup our mediamosa connection.
            global $user;

            // Create unique pass.
            $csa_id = uniqid(mt_rand(), TRUE);
            // only available for some period.
            variable_set('mediamosa_completed_transcoding_url_' . $csa_id, array('mediafile_ids_to_remove' => $mediafile_ids_to_remove, 'user_id' => $user->uid, 'start' => microtime(TRUE)));

            // Setup the completed_transcode_url.
            $url_options = array(
              'absolute' => TRUE,
              'query' => array('csa_id' => $csa_id),
            );

            $options['completed_transcoding_url'] = url('csa/box/batch/transcode_completed', $url_options);
          }

        case CSA_TRANSCODE_REPLACE_OPTION_ADD:
          break;
      }

      // If we found existing mediafiles with same profile ID and we selected skip, then skip the transcode.
      if ($context['results']['options']['transcode_replace'] == CSA_TRANSCODE_REPLACE_OPTION_SKIP && !empty($mediafile_ids_to_remove)) {
        continue;
      }

      // Call transcode rest call.
      _csa_asset_mediafile_transcode($asset['mediafile_original']['mediafile_id'], $user_id, $profile_id, $options);
    }

    // Set the title.
    $context['message'] = t("Started transcode on asset '@title'", array('@title' => _csa_asset_get_title($asset)));
  }

  // Add the asset ID for result.
  $context['results']['count']++;
}

/**
 * Triggered when transcode was completed on mediafile.
 */
function _csa_box_batch_transcode_completed_trigger() {
  $csa_id = empty($_REQUEST['csa_id']) ? '' : $_REQUEST['csa_id'];
  $status_code = empty($_REQUEST['status_code']) ? '' : $_REQUEST['status_code'];

  if ($status_code != 'FINISHED') {
    exit(); // wait for it.
  }

  // Only when it
  $stuff = empty($csa_id) ? '' : variable_get('mediamosa_completed_transcoding_url_' . $csa_id, '');

  if (empty($stuff)) {
    _csa_log_critical('Transcode completed triggered with missing or unknown ID.');
    exit();
  }
  else {
    // Used once.
    variable_del('mediamosa_completed_transcoding_url_' . $csa_id);
  }

  // Get the IDs.
  $mediafile_ids_to_remove = $stuff['mediafile_ids_to_remove'];
  $user_id = $stuff['user_id'];

  // Get user.
  $account = user_load(intval($user_id));
  if ($account == FALSE) {
    _csa_log_critical('_csa_box_batch_transcode_completed_trigger(): User account (ID: @user_id was not found.', array('@user_id' => $user_id));
    exit();
  }

  // Got account, now make if the global user.
  global $user;
  $user = $account; // mediamosa connector will now use this user's login.

  // Log it.
  _csa_log_debug('Got transcode completed trigger, mediafiles to remove; @mediafile_ids_to_remove. For user with ID: @user_id.', array('@mediafile_ids_to_remove' => implode(', ', $mediafile_ids_to_remove), '@user_id' => $user_id));

  // Now remove them.
  foreach ($mediafile_ids_to_remove as $mediafile_id) {
    // Get it first.
    $mediafile = _csa_asset_mediafile_get($mediafile_id, array('show_stills' => FALSE));
    $mediafile = reset($mediafile['items']['item']);

    // Make sure we don't delete the original!
    if (drupal_strtoupper($mediafile['is_original_file']) == 'TRUE') {
      _csa_log_critical('Tried to delete original mediafile!');
      continue;
    }

    // Now delete the mediafile.
    _csa_asset_mediafile_delete($mediafile['mediafile_id'], $mediafile['owner_id']);
  }
}

/**
 * Batch for moving assets into collections.
 *
 * @param string $type
 *   unpublished/published.
 */
function _csa_box_batch_move_into_collection($type, $coll_id) {

  // Get the collection.
  $collection = _csa_collection_get($coll_id, array('fatal' => TRUE));
  $name = _csa_collection_get_title($collection);

  _csa_box_batch(
    url('csa/' . $type),
    array(
      'function_name' => '_csa_box_batch_move_asset_into_collection',
      'type' => $type,
      'count' => 100, // Per 100 assets.
      'success_message' => array(
        'singular' => strtr("Added 1 asset to collection '@name'.", array('@name' => $name)),
        'plural' => strtr("Added @count assets to collection '@name'.", array('@name' => $name)),
      ),
      'failure_message' => strtr("Failed to add @failed assets of the @total assets in collection '@name'.", array('@name' => $name)),
      'collection' => $collection,
      'collection_name' => $name,
    ),
    array(
      'title' => t("Adding assets to collection '@name'", array('@name' => $name)),
      'init_message' => t("Starting batch to add asset(s) to collection '@name'.", array('@name' => $name)),
      'progress_message' => t('Adding @current asset out of @total  (@remaining remaining).'),
      'error_message' => t("Adding asset(s) in collection '@name' batch has encountered an error.", array('@name' => $name)),
    )
  );
}

/**
 * Function called within the batch.
 *
 * @param array $context
 * @param string $type
 * @param string $mediafile_id
 */
function _csa_box_batch_move_asset_into_collection(&$context, $type, $asset_ids) {

  assert(isset($context['results']['options']['collection']));
  $asset_ids = is_array($asset_ids) ? $asset_ids : array($asset_ids);

  // Get the owner of the collection.
  $user_id = $context['results']['options']['collection']['owner_id'];

  // Get coll_id.
  $coll_id = $context['results']['options']['collection']['coll_id'];

  // Move into collection.
  _csa_collection_add_assets($asset_ids, $coll_id, $user_id, array('fatal' => TRUE));

  // Add the asset ID for result.
  $context['results']['count'] += count($asset_ids);

  // Set the title.
  $context['message'] = t("Added @count assets to collection '@name'", array('@count' => count($asset_ids), '@name' => $context['results']['options']['collection_name']));
}

/**
 * Batch for moving assets into collections.
 *
 * @param string $type
 *   unpublished/published.
 */
function _csa_box_batch_remove_from_collection($type, $coll_id) {

  // Get the collection.
  $collection = _csa_collection_get($coll_id, array('fatal' => TRUE));
  $name = _csa_collection_get_title($collection);

  _csa_box_batch(
    url('csa/' . $type),
    array(
      'function_name' => '_csa_box_batch_remove_asset_from_collection',
      'type' => $type,
      'count' => 100, // Per 100 assets.
      'success_message' => array(
        'singular' => strtr("Removed 1 asset from collection '@name'.", array('@name' => $name)),
        'plural' => strtr("Removed @count assets from collection '@name'.", array('@name' => $name)),
      ),
      'failure_message' => strtr("Failed to remove @failed assets of the @total assets from collection '@name'.", array('@name' => $name)),
      'collection' => $collection,
      'collection_name' => $name,
    ),
    array(
      'title' => t("Removing assets from collection '@name'", array('@name' => $name)),
      'init_message' => t("Starting batch to remove asset(s) from collection '@name'.", array('@name' => $name)),
      'progress_message' => t('Removed @current asset out of @total  (@remaining remaining).'),
      'error_message' => t("Removing asset(s) from collection '@name' batch has encountered an error.", array('@name' => $name)),
    )
  );
}

/**
 * Function called within the batch.
 *
 * @param array $context
 * @param string $type
 * @param string $mediafile_id
 */
function _csa_box_batch_remove_asset_from_collection(&$context, $type, $asset_ids) {

  assert(isset($context['results']['options']['collection']));
  $asset_ids = is_array($asset_ids) ? $asset_ids : array($asset_ids);

  // Get the owner of the asset.
  $user_id = $context['results']['options']['collection']['owner_id'];

  // Get coll_id.
  $coll_id = $context['results']['options']['collection']['coll_id'];

  // Remove from collection. Returns the number of assets really removed.
  $context['results']['count'] += _csa_collection_remove_assets($asset_ids, $coll_id, $user_id, array('fatal' => TRUE));

  // Set the title.
  $context['message'] = t("Removed @count assets to collection '@name'", array('@count' => count($asset_ids), '@name' => $context['results']['options']['collection_name']));
}

/**
 * Batch for generating stills.
 */
function _csa_box_batch_generate_still($type, $still_options) {
  global $user;

  $still_options += array(
    'still_type' => CSA_STILL_TYPE_NONE,
    'still_per_mediafile' => CSA_STILL_PER_MEDIAFILE,
    'still_every_second'=> CSA_STILL_EVERY_SECOND,
    'start_time' => CSA_STILL_START_TIME,
    'end_time' => CSA_STILL_END_TIME,
    'size' => CSA_STILL_SIZE,
    'h_padding' => CSA_STILL_H_PADDING,
    'v_padding' => CSA_STILL_V_PADDING,
  );

  _csa_box_batch(
    url('csa/' . $type),
    array(
      'function_name' => '_csa_box_batch_generate_still_asset',
      'type' => $type,
      'success_message' => array(
        'singular' => 'Added still(s) on 1 asset.',
        'plural' => 'Added still(s) on @count assets.',
      ),
      'failure_message' => 'Failed to add still(s) on @failed assets of the @total assets.',
      'still_options' => $still_options,
    ),
    array(
      'title' => t('Adding still(s) on assets'),
      'init_message' => t('Starting batch for adding still(s) to selected assets.'),
      'progress_message' => t('Adding still(s) @current asset out of @total  (@remaining remaining).'),
      'error_message' => t('Adding still(s) batch has encountered an error.'),
    )
  );
}

/**
 * Common metadata.
 *
 * @param array $context
 * @param string $type
 * @param string $asset_id
 */
function _csa_box_batch_generate_still_asset(&$context, $type, $asset_id) {
  // Get asset.
  $asset = _csa_box_batch_get_asset($asset_id, $context);

  if ($asset) {
    // Get the original mediafile.
    // Find the original.
    if (!empty($asset['mediafiles']['mediafile'])) {
      foreach ($asset['mediafiles']['mediafile'] as $key => $mediafile) {
        unset($asset['mediafiles']['mediafile'][$key]['@attributes']);

        if (drupal_strtoupper($mediafile['is_original_file']) == 'TRUE') {
          // Set the original mediafile id for this asset.
          $asset['mediafile_original'] = $mediafile;
        }
      }
    }

    if (!isset($asset['mediafile_original'])) {
      // If we can't find the original, then throw exception.
      throw new Exception(t('Asset with ID @asset_id (@title) does not have an original mediafile, can not create still(s) with out original.', array('@asset_id' => $asset_id, '@title' => _csa_asset_get_title($asset))));
    }

    // Get the owner of the mediafile.
    $user_id = $asset['mediafile_original']['owner_id'];

    // Get the mediafile_id of the original.
    $mediafile_id = $asset['mediafile_original']['mediafile_id'];

    // Copy the still settings.
    $options = $context['results']['options']['still_options'];
    $options['fatal'] = TRUE;

    // Save metadata.
    _csa_still_create($mediafile_id, $user_id, $options);

    // Set the title.
    $context['message'] = t("Added still(s) to asset '@title'", array('@title' => _csa_asset_get_title($asset)));
  }

  // Add the asset ID for result.
  $context['results']['count']++;
}

/**
 * Batch for moving assets into collections.
 *
 * @param string $type
 *   unpublished/published.
 */
function _csa_box_batch_access_rules($type, $access) {

  _csa_box_batch(
    url('csa/' . $type),
    array(
      'function_name' => '_csa_box_batch_access_rules_asset',
      'type' => $type,
      'success_message' => array(
        'singular' => "Added access rules to 1 asset.",
        'plural' => "Added access rules to @count assets.",
      ),
      'failure_message' => "Failed to add access rules to @failed assets of the @total assets.",
      // ACL settings.
      'aut_app' => array(), //array_keys($user->csa_access_rules['applications']),
      'aut_domain' => $access['domains'],
      'aut_realm' => $access['realms'],
    ),
    array(
      'title' => t("Adding access rules"),
      'init_message' => t("Starting to add access rules."),
      'progress_message' => t('Added access rules to @current asset out of @total  (@remaining remaining).'),
      'error_message' => t("Adding access rules has encountered an error."),
    )
  );
}

/**
 * Common metadata.
 *
 * @param array $context
 * @param string $type
 * @param string $asset_id
 */
function _csa_box_batch_access_rules_asset(&$context, $type, $asset_id) {
  // Get asset.
  $asset = _csa_box_batch_get_asset($asset_id, $context);

  if ($asset) {

    // All ok, now publish.
    $options = array(
      'aut_app' => $context['results']['options']['aut_app'],
      'aut_domain' => $context['results']['options']['aut_domain'],
      'aut_realm' => $context['results']['options']['aut_realm'],
      'fatal' => TRUE,
    );

    // Now loop through the mediafiles (if any) and set ACL.
    foreach ($asset['mediafiles']['mediafile'] as $mediafile) {

      // Get ACL.
      $rights = _csa_asset_mediafile_get_acl($mediafile['mediafile_id'], $mediafile['owner_id']);

      $options['aut_app'] = array();

      foreach ($rights as $right) {
        if (!empty($right['aut_app'])) {
          $options['aut_app'][] = $right['aut_app'];
        }
      }

      _csa_asset_mediafile_set_acl($mediafile['mediafile_id'], $mediafile['owner_id'], $options);
    }
  }
}
