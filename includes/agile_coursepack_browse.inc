<?php

function agile_coursepack_browse() {
  module_load_include('inc', 'islandora_solr', 'includes/block');
  $block = module_invoke('islandora_solr', 'block_view', 'simple');
  $content = render($block['content']);
  return $content;
}
