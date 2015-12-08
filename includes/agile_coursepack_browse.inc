<?php

/**
 * Interface to allow browsing of content only.
 *
 * @return string
 */

/**
 * Searches content filtered by content
 * @return string
 */
function agile_coursepack_browse() {
  global $_islandora_solr_queryclass;
  module_load_include('inc', 'islandora_solr', 'includes/block');
  module_load_include('inc', 'agile_coursepack', 'includes/coursepack_splash');
  $module_path = drupal_get_path('module', 'agile_coursepack');
  drupal_add_css("$module_path/css/agile_coursepack_splash.css");
  $block = module_invoke('islandora_solr', 'block_view', 'simple');
  $sort = module_invoke('islandora_solr', 'block_view', 'sort');
  $content = render($sort['content']);
  $content .= render($block['content']);
  $path_parts = explode('/', $_GET['q']);
  $query = '%20';
  if (isset($path_parts[2])) {
    $query = $path_parts[2];
  }
  $content .= islandora_solr($query);
  $buttons = drupal_get_form('agile_coursepack_button_form');
  $content = drupal_render($buttons) . $content;
  return $content;
}

function agile_coursepack_button_form($form, $form_state) {
  module_load_include('inc', 'agile_coursepack', 'includes/coursepack_splash');
  if (isset($form_state['triggering_element'])) {
    if ($form_state['triggering_element']['#id'] == 'agile-coursepack-add') {
      drupal_goto('agile_coursepack/add_item');
    }
    if ($form_state['triggering_element']['#id'] == 'agile-coursepack-browse') {
      drupal_goto('agile_coursepack/browse');
    }
    if ($form_state['triggering_element']['#id'] == 'agile-coursepack-create') {
      drupal_goto('agile_coursepack/add_coursepack');
    }
    if ($form_state['triggering_element']['#id'] == 'agile-coursepack-coursepacks') {
      drupal_goto('agile_coursepack/coursepacks');
    }
  }
  $form = agile_coursepack_buttons();
  return $form;
}

/**
 * Searches content filtered by content
 * @return string
 */
function agile_coursepack_coursepack_browse() {
  module_load_include('inc', 'islandora_solr', 'includes/block');
  module_load_include('inc', 'agile_coursepack', 'includes/coursepack_splash');
  $block = module_invoke('islandora_solr', 'block_view', 'advanced');
  $content = render($block['content']);
  $path_parts = explode('/', $_GET['q']);
  $query = '%2A%3A%2A';
  if (isset($path_parts[2])) {
    $query = $path_parts[2];
  }
  $content .= islandora_solr($query);
  return $content;
}
