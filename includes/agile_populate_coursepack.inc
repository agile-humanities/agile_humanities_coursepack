<?php

function agile_populate_coursepack($pid) {
  module_load_include('inc', 'islandora_solr', 'includes/block');
  module_load_include('inc', 'agile_coursepack', 'includes/coursepack_splash');
  $module_path = drupal_get_path('module', 'agile_coursepack');
  drupal_add_css("$module_path/css/agile_coursepack_splash.css");
  $block = module_invoke('islandora_solr', 'block_view', 'simple');
  $content = render($block['content']);
  $path_parts = explode('/', $_GET['q']);
  if (!isset($_GET['hidden_filter'])) {
    $_GET['hidden_filter'] = array('');
  }
  else {
    $_GET['hidden_filter'][0] .= ' || ';
  }
  $content_models = array(
    'info:fedora/islandora:sp_videoCModel',
    'info:fedora/islandora:sp-audioCModel',
    'info:fedora/islandora:sp_basic_image',
    'info:fedora/islandora:sp_large_image_cmodel',
    'info:fedora/islandora:sp_document',
  );
  $content_models_string = implode('" || "', $content_models);
  $_GET['hidden_filter'][0] .= 'RELS_EXT_hasModel_uri_ms:("' . $content_models_string . '")';
  $_GET['display'] = 'coursepack';
  $query = '%20';
  if (isset($path_parts[2])) {
    $query = $path_parts[2];
  }
  $content .= islandora_solr($query);

  $buttons = drupal_get_form('agile_coursepack_button_form');
  $content = drupal_render($buttons) . $content;
  return $content;
}
