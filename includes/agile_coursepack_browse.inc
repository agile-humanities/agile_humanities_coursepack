<?php

function agile_coursepack_browse() {
  module_load_include('inc', 'islandora_solr', 'includes/block');
  module_load_include('inc', 'agile_coursepack', 'includes/coursepack_splash');
  $block = module_invoke('islandora_solr', 'block_view', 'simple');
  $content = render($block['content']);
  $path_parts = explode('/', $_GET['q']);
  $query = '%20';
  if (isset($path_parts[2])) {
    $query = $path_parts[2];
  }
  $content .= islandora_solr($query);
  
  $buttons = agile_coursepack_buttons();
  $content = drupal_render($buttons) . $content;
  return $content;
}
