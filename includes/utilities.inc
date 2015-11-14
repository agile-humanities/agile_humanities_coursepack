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