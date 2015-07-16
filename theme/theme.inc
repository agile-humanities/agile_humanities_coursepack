<?php

/**
 * @file
 * This file contains all theme and preprocess functions.
 */

/**
 * Implements hook_preprocess_theme().
 */
function agile_coursepack_preprocess_agile_coursepack(array &$variables) {
  $object = $variables['islandora_object'];
  $block = module_invoke('agile_coursepack', 'block_view', 'coursepack_solr');
  $variables['search_block'] = render($block['content']);
  if (islandora_object_access_callback(ISLANDORA_METADATA_EDIT, $object)) {
    $variables['metadata_link'] = l(t("Edit metadata"), "islandora/edit_form/{$object->id}/MODS");
    $variables['manage_link'] = l(t("Manage members"), "islandora/object/{$object->id}/manage/compound");
  }
  $variables['document'] = theme('islandora_document', array('islandora_object' => $object));
}