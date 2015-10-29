<?php

function agile_coursepack_browse() {
  module_load_include('inc', 'islandora_solr', 'includes/block');
  module_load_include('inc', 'agile_coursepack', 'includes/coursepack_splash');
  $module_path = drupal_get_path('module', 'agile_coursepack');
  drupal_add_css("$module_path/css/agile_coursepack_splash.css");
  $block = module_invoke('islandora_solr', 'block_view', 'simple');
  $content = render($block['content']);
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
  }
  $form = agile_coursepack_buttons();
  return $form;
}
