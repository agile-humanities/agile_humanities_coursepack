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
