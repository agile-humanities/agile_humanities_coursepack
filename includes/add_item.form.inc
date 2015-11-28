<?php

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
function agile_coursepack_add_item_form(array $form, array &$form_state) {
  module_load_include('inc', 'agile_coursepack', 'includes/utilities');
  $module_path = drupal_get_path('module', 'agile_coursepack');

  $form['author_first'] = array(
    '#type' => 'textfield',
    '#title' => t('Creator First Name'),
    '#description' => t("Creator first name (Author, painter, etc)"),
  );
  $form['author_last'] = array(
    '#type' => 'textfield',
    '#title' => t('Creator Last Name'),
    '#description' => t('Creator last name (Author, painter, etc)'),
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title of work'),
  );
  $form['dateIssued'] = array(
    '#type' => 'date_popup',
    '#title' => t('First publication'),
    '#date_format' => 'Y/m/d',
  );
  $form['country'] = array(
    '#type' => 'textfield',
    '#title' => t('Country'),
    '#description' => t('Country of first publication'),
  );
  $form['publisher'] = array(
    '#type' => 'textfield',
    '#title' => t('Publisher'),
  );
  $form['genre'] = array(
    '#type' => 'textfield',
    "#title" => t('Genre'),
    '#description' => t('(Poem, novel,manifesto, etc.)'),
  );
  $form['language'] = array(
    '#type' => 'textfield',
    "#title" => t('Language'),
    '#description' => t('Publication language'),
  );
  $form['manifestation'] = array(
    '#type' => 'textfield',
    '#title' => t('Manifestation'),
    '#description' => t('(periodical, book, broadside, etc.)'),
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

  $form['source_file'] = array(
    '#title' => t('Source file'),
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
    '#value' => t("Ingest Digital Asset"),
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
function agile_coursepack_add_item_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'agile_coursepack', 'includes/utilities');
  $mappings = coursepack_get_extensions();
  $content_model_mappings = array(
    'video' => 'islandora:sp_videoCModel',
    'audio' => 'islandora:sp-audioCModel',
    'image' => 'islandora:sp_basic_image',
    'large_image' => 'islandora:sp_large_image_cmodel',
    'pdf' => 'islandora:sp_document',
  );
  $collection_mappings = array(
    'video' => 'islandora:video_collection',
    'audio' => 'islandora:audio_collection',
    'image' => 'islandora:sp_basic_image_collection',
    'large_image' => 'islandora:sp_large_image_collection',
    'pdf' => 'islandora:sp_document_collection',
  );
  $tuque = new IslandoraTuque();
  $namespace = variable_get('agile_coursepack_object_namespace', 'islandora');
  $object = $tuque->repository->constructObject($namespace);
  $object->label = $form_state['values']['title'];
  // Create and add MODS
  $mods = coursepack_build_mods($form_state['values']);
  $mods_ds = $object->constructDatastream('MODS', 'M');
  $mods_ds->setContentFromString($mods);
  $mods_ds->label = 'MODS';
  $mods_ds->mimetype = 'text/xml';
  $object->ingestDatastream($mods_ds);
  // Add primary resource
  $source_file = file_load($form_state['values']['source_file']);
  $ds = $object->constructDatastream('OBJ', 'M');
  $ds->setContentFromFile(drupal_realpath($source_file->uri), FALSE);
  $ds->label = 'OBJ';
  $ds->mimetype = $source_file->filemime;
  $object->ingestDatastream($ds);

  $path_parts = pathinfo($source_file->uri);
  $extension = strtolower($path_parts['extension']);
  foreach ($mappings as $type => $extension_array) {
    if (in_array($extension, $extension_array)) {
      $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', $content_model_mappings[$type]);
      $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOf', $collection_mappings[$type]);

      break;
    }
  }
  $ingested_object = islandora_add_object($object);
  drupal_set_message(t('Ingested @label', array('@label' => $ingested_object->label)));
}

/**
 * Builds MODS data.
 *
 * @param array $inputs
 *   Input values from $form_state['values']
 *
 * @return xml
 *   Formatted xml
 */
function coursepack_build_mods($inputs) {
  $xml = new DomDocument('1.0', 'UTF-8');
  $xml->preserveWhiteSpace = false;
  $xml->formatOutput = true;
  $mods_uri = "http://www.loc.gov/mods/v3";
  //root.
  $root = $xml->createElementNS($mods_uri, 'mods:mods');
  $xml->appendChild($root);
  add_title($xml, $inputs['title']);
  $name = "{$inputs['author_last']}, {$inputs['author_first']}";
  add_authority($xml, 'authority', $name);
  add_originInfo($xml, $inputs);
  add_language($xml, $inputs['language']);

  return $xml->saveXML();
}

/**
 * Adds authority record.
 *
 * @param DomDocument $xml
 *   xml document
 * @param string $type
 *   Authority type
 * @param string $name
 *   value for authority
 */
function add_authority($xml, $type, $name) {
  $mods_uri = "http://www.loc.gov/mods/v3";
  $root = $xml->documentElement;
  // name.
  $name_node = $xml->createElementNS($mods_uri, 'mods:name');
  $name_node->setAttribute('type', 'personal');
  $root->appendChild($name_node);
  //namePart
  $name_part = $xml->createElementNS($mods_uri, 'mods:namePart', $name);
  $name_node->appendChild($name_part);
  //role
  $role_node = $xml->createElementNS($mods_uri, 'mods:role');
  $name_node->appendChild($role_node);
  $roleTerm_node = $xml->createElementNS($mods_uri, 'mods:roleTerm', "Author");
  $roleTerm_node->setAttribute('authority', 'marcrelator');
  $roleTerm_node->setAttribute('type', 'text');
  $role_node->appendChild($roleTerm_node);
}

/**
 * Adds title to document.
 *
 * @param DomDocument $xml
 *   DomDocument to which values are added
 * @param string $title
 *   Title to be added
 */
function add_title($xml, $title) {
  $mods_uri = "http://www.loc.gov/mods/v3";
  $root = $xml->documentElement;
  $titleInfo_node = $xml->createElementNS($mods_uri, "mods:titleInfo");
  $root->appendChild($titleInfo_node);
  $title_node = $xml->createElementNS($mods_uri, 'mods:title', $title);
  $titleInfo_node->appendChild($title_node);
}

/**
 * Adds origin info to document.
 *
 * @param DomDocument $xml
 *   DomDocument to which values are added
 * @param array $values
 *   OriginInfo values to be added
 */
function add_originInfo($xml, $values) {
  $mods_uri = "http://www.loc.gov/mods/v3";
  $root = $xml->documentElement;
  $originInfo_node = $xml->createElementNS($mods_uri, "mods:originInfo");
  $root->appendChild($originInfo_node);
  if (isset($values['dateIssued'])) {
    $dateIssued_node = $xml->createElementNS($mods_uri, 'mods:dateIssued', $values['dateIssued']);
    $originInfo_node->appendChild($dateIssued_node);
  }
  if (isset($values['publisher'])) {
    $publisher_node = $xml->createElementNS($mods_uri, 'mods:publisher', $values['publisher']);
    $originInfo_node->appendChild($publisher_node);
  }
  if (isset($values['country'])) {
    $place_node = $xml->createElementNS($mods_uri, 'mods:place');
    $originInfo_node->appendChild($place_node);
    $placeTerm_node = $xml->createElementNS($mods_uri, 'mods:placeTerm', $values['country']);
    $placeTerm_node->setAttribute('authority', 'marccountry');
    $place_node->appendChild($placeTerm_node);
  }
}

/**
 * Adds language to document.
 *
 * @param DomDocument $xml
 *   DomDocument to which values are added
 * @param string $language
 *   Language to be added
 */
function add_language($xml, $language) {
  $mods_uri = "http://www.loc.gov/mods/v3";
  $root = $xml->documentElement;
  $language_node = $xml->createElementNS($mods_uri, "mods:language");
  $root->appendChild($language_node);
  $languageTerm_node = $xml->createElementNS($mods_uri, 'mods:languageTerm', $language);
  $languageTerm_node->setAttribute('authority', 'iso639-2b');
  $languageTerm_node->setAttribute('type', 'code');
  $language_node->appendChild($languageTerm_node);
}
