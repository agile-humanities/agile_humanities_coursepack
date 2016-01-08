<?php

/**
 * @file
 * Utiltiy functions.
 */

/**
 * Gets object pid from URL.
 *
 * @return mixed
 *   PID if found, FALSE if not.
 */
function agile_coursepack_get_pid_from_url() {
  $is_coursepack = FALSE;
  $object = NULL;
  if (isset($_GET['q'])) {
    $path_parts = explode('/', $_GET['q']);
    if (isset($path_parts[2])) {
      $pid = $path_parts[2];
      $object = islandora_object_load($pid);
    }
    if (!$object) {
      $pid = isset($_GET['pid']) ? $_GET['pid'] : FALSE;
      $object = islandora_object_load($pid);
    }
    if ($object) {
      if (in_array('islandora:coursepackCModel', $object->models)) {
        $is_coursepack = TRUE;
        $form_state['coursepack_pid'] = $object->id;
      }
    }
  }
  return $is_coursepack ? $object->id : FALSE;
}

/**
 * Gets all allowable extensions.
 *
 * @return array
 *   Allowable extensions
 */
function coursepack_get_extensions() {
  $audio_extensions = array('wav', 'mp3');
  $large_image_extensions = array('tif', 'tiff');
  $image_extensions = array('jpg', 'jpeg', 'gif', 'png');
  $video_extensions = array('ogg', 'mp4', 'mov', 'qt', 'm4v', 'avi', 'mkv');
  module_load_include('inc', 'islandora_jodconverter', 'includes/utilities');
  $pdf = islandora_jodconverter_get_format('pdf');
  $pdf['from'][] = 'pdf';
  return array(
    'audio' => $audio_extensions,
    'video' => $video_extensions,
    'pdf' => $pdf['from'],
    'large_image' => $large_image_extensions,
    'image' => $image_extensions,
  );
}

/**
 * Downloads all associated pdfs.
 *
 * @param string $pid
 *   Coursepack PID
 */
function agile_coursepack_download($pid) {
  $pack = islandora_object_load($pid);
  $children = islandora_compound_object_get_parts($pid, TRUE);
  $pids = array_keys($children);
  $zip = new ZipArchive();
  $tmp_file = drupal_realpath(tempnam('temporary://', "agile_coursepack_"));
  $zip->open($tmp_file, ZipArchive::CREATE);
  $counter = 1;
  foreach ($pids as $pid) {
    $object = islandora_object_load($pid);
    $stream = $object['PDF'] ? 'PDF' : 'OBJ';
    $extension = 'pdf';
    if ($stream == 'OBJ') {
      $mime_type = $object['OBJ']->mimeType;
      $mime_detect = new MimeDetect();
      $extension = $mime_detect->getExtension($mime_type);
    }
    $clean_name = str_pad($counter, 2, "00", STR_PAD_LEFT);
    $clean_name .= "-" . str_replace(' ', '_', $object->label) . ".$extension";
    $filename = drupal_realpath(file_create_filename("$clean_name", 'temporary://'));
    $contents = $object[$stream]->content;
    file_put_contents($filename, $contents, FILE_APPEND | LOCK_EX);
    $realpath = drupal_realpath($filename);
    $success = $zip->addFile($realpath, $clean_name);
    $counter++;
  }
  $zip->close();
  $download = str_replace(' ', '_', $pack->label) . '.zip';
  header("Content-disposition: attachment; filename=$download");
  header('Content-type: application/zip');
  readfile($tmp_file);
}

/**
 * Retrieves and allows selection of content for coursepacks.
 * 
 * @param string $pid
 *   PID of coursepack
 *
 * @return html
 *   Styled output
 */
function agile_populate_coursepack($pid) {
  module_load_include('inc', 'islandora_solr', 'includes/block');
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

  return $content;
}

function agile_coursepack_solr_redirect($coursepack_pid) {
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
  module_load_include('inc', 'agile_coursepack', 'includes/utilities');
  $search_string = '%20';
  $options = array('query' => array('type' => 'dismax'));
  $options['query']['display'] = 'coursepack';
  $options['query']['pid'] = $coursepack_pid;
  $options['query']['hidden_filter'][0] = 'RELS_EXT_hasModel_uri_ms:("' . $content_models_string . '")';
  drupal_goto('islandora/search/' . $search_string, $options);
}
