<?php

/**
 * @file
 * Create coursepack forms.
 */

/**
 * Coursepack's add item form.
 *
 * @param array $form
 *   Drupal Form
 * @param array $form_state
 *   Drupal Form State
 *
 * @return array
 *   The drupal form definition.
 */
function agile_coursepack_add_coursepack_form(array $form, array &$form_state, $pid = NULL) {
  module_load_include('inc', 'agile_coursepack', 'includes/coursepack_datastream');
  $new = TRUE;
  $needs_description = TRUE;
  if ($pid) {
    $coursepack = islandora_object_load($pid);
    $cp = new CPData($coursepack['CPDATA']->content);
    $cp_values = $cp->cp;
    $new = FALSE;
    $form['pid'] = array(
      '#type' => 'hidden',
      '#value' => $pid,
    );
    if ($coursepack['OBJ']) {
      $needs_description = FALSE;
    }
  }
  module_load_include('inc', 'agile_coursepack', 'includes/utilities');
  $module_path = drupal_get_path('module', 'agile_coursepack');
  if ($new) {
    $licence = l(t('Creative Commons Attribution-NonCommercial 4.0 International License.'), 'http://creativecommons.org/licenses/by-nc/4.0/');
    $form['license'] = array(
      '#type' => 'markup',
      '#markup' => t('All coursepacks will be created with a !license', array('!license' => "<br />$licence")),
    );
  }
  $form['new'] = array(
    '#type' => 'hidden',
    '#value' => $new,
  );

  $form['course_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Course Name'),
    '#default_value' => ($new) ? '' : $cp_values['cp:name'],
  );
  $form['course_number'] = array(
    '#type' => 'textfield',
    '#title' => t('Course number'),
    '#default_value' => ($new) ? '' : $cp_values['cp:number'],
  );
  $form['year'] = array(
    '#type' => 'textfield',
    '#title' => t('Year'),
    '#default_value' => ($new) ? '' : $cp_values['cp:year'],
  );
  $form['term'] = array(
    '#type' => 'textfield',
    '#title' => t('Term'),
    '#default_value' => ($new) ? '' : $cp_values['cp:term'],
  );
  $form['first'] = array(
    '#type' => 'textfield',
    '#title' => t('Instructor First Name'),
    '#default_value' => ($new) ? '' : $cp_values['cp:instructorfirst'],
  );
  $form['last'] = array(
    '#type' => 'textfield',
    '#title' => t('Instructor Last Name'),
    '#default_value' => ($new) ? '' : $cp_values['cp:instructorlast'],
  );
  $form['honorific'] = array(
    '#type' => 'textfield',
    '#title' => t('Instructor Title'),
    '#default_value' => ($new) ? '' : $cp_values['cp:instructortitle'],
  );
  $form['dept'] = array(
    '#type' => 'textfield',
    '#title' => t('Department'),
    '#default_value' => ($new) ? '' : $cp_values['cp:department'],
  );
  $form['institution'] = array(
    '#type' => 'textfield',
    '#title' => t('Institution'),
    '#default_value' => ($new) ? '' : $cp_values['cp:institution'],
  );

  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $audio_extensions = 'wav mp3';
  $video_extensions = 'ogg mp4 mov qt m4v avi mkv';
  module_load_include('inc', 'islandora_jodconverter', 'includes/utilities');
  $pdf = islandora_jodconverter_get_format('pdf');
  $pdf['from'][] = 'pdf';
  $pdf_extensions = implode(' ', $pdf['from']);
  $extensions = coursepack_get_extensions();
  $allowed = '';
  foreach ($extensions as $key => $extension) {
    $allowed .= ' ' . implode(' ', $extension);
  }
  $title = $needs_description ? t('Add course description') : t('Update course description');
  $form['source_file'] = array(
    '#title' => $title,
    '#type' => 'managed_file',
    '#required' => FALSE,
    '#description' => t('Select file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $allowed)),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => array($allowed),
      // Assume it's specified in MB.
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Create coursepack"),
  );

  $label = $new ? t("Create coursepack") : t("Update course details");
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $label,
  );
  return $form;
}

/**
 * Creates defined object.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function agile_coursepack_add_coursepack_form_submit(array $form, array &$form_state) {
  if ($form_state['values']['new']) {
    $tuque = new IslandoraTuque();
    $namespace = variable_get('agile_coursepack_object_namespace', 'islandora');
    $object = $tuque->repository->constructObject($namespace);
    $object->label = $form_state['values']['course_name'];
    // Create and add CP Data
    $cp_data = agile_coursepack_make_cpdata($form_state['values']);
    $cpdata_ds = $object->constructDatastream('CPDATA', 'X');
    $cpdata_ds->setContentFromString($cp_data);
    $cpdata_ds->label = 'CPData';
    $cpdata_ds->mimetype = 'text/xml';
    $object->ingestDatastream($cpdata_ds);
    // Add primary resource
    if ($form_state['values']['source_file']) {
      $source_file = file_load($form_state['values']['source_file']);
      $ds = $object->constructDatastream('OBJ', 'M');
      $ds->setContentFromFile(drupal_realpath($source_file->uri), FALSE);
      $ds->label = 'OBJ';
      $ds->mimetype = $source_file->filemime;
      $object->ingestDatastream($ds);
    }
    $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:coursepackCModel');
    $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', 'islandora:coursepacks');
    $ingested_object = islandora_add_object($object);
    drupal_set_message(t('Ingested @label', array('@label' => $ingested_object->label)));
    $form_state['redirect'] = "islandora/object/$ingested_object->id";
  }
  else {
    $object = islandora_object_load($form_state['values']['pid']);
    $soource_file = NULL;
    $new_obj = $object['OBJ'] ? FALSE : TRUE;
    $cp_data = agile_coursepack_make_cpdata($form_state['values']);
    $object['CPDATA']->content = $cp_data;
    if ($form_state['values']['source_file']) {
      $source_file = file_load($form_state['values']['source_file']);
    }
    if ($source_file) {
      if ($new_obj) {
        $ds = $object->constructDatastream('OBJ', 'M');
        $ds->setContentFromFile(drupal_realpath($source_file->uri), FALSE);
        $ds->label = 'OBJ';
        $ds->mimetype = $source_file->filemime;
        $object->ingestDatastream($ds);
      }
      else {
        $object['OBJ']->setContentFromFile(drupal_realpath($source_file->uri), FALSE);
        if ($object['OBJ']->mimetype != $source_file->filemime) {
          $object['OBJ']->mimetype = $source_file->filemime;
        }
      }
    }


    $form_state['redirect'] = "islandora/object/$object->id";
  }
}

function agile_coursepack_make_cpdata($values) {
  module_load_include('inc', 'agile_coursepack', 'includes/coursepack_datastream');
  $cp = new CPData();
  $cp->addElement('cp:name', $values['course_name']);
  $cp->addElement('cp:number', $values['course_number']);
  $cp->addElement('cp:year', $values['year']);
  $cp->addElement('cp:term', $values['term']);
  $cp->addElement('cp:instructorfirst', $values['first']);
  $cp->addElement('cp:instructorlast', $values['last']);
  $cp->addElement('cp:instructortitle', $values['honorific']);
  $cp->addElement('cp:department', $values['dept']);
  $cp->addElement('cp:institution', $values['institution']);
  return $cp->asXML();
}

function agile_upsert_description($object, $source_file) {
  $existing = $object['OBJ'];
  $ds = $existing ? $object->constructDatastream('OBJ', 'M') : $object['OBJ'];
  $ds->setContentFromFile(drupal_realpath($source_file->uri), FALSE);
  if (!$ds->label) {
    $ds->label = 'OBJ';
  }
  if (!$ds->mimetype || $ds->mimetype == $source_file->filemime) {
    $ds->mimetype = $source_file->filemime;
  }
  if (!$existing) {
    $object->ingestDatastream($ds);
  }
}
