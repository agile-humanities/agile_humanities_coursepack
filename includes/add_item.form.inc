<?php

/**
 * @file
 * Form definition to add ne items.
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
function agile_coursepack_add_item_form(array $form, array &$form_state, $pid = NULL) {
  module_load_include('inc', 'agile_coursepack', 'includes/utilities');
  $module_path = drupal_get_path('module', 'agile_coursepack');
  if ($pid) {
    $object = islandora_object_load($pid);
    $results = agile_coursepack_info_fields($object);
    $name_parts = explode(',', $results['name'][0]);
    $results['first_name'] = $name_parts[1];
    $results['last_name'] = $name_parts[0];
    $form_state['pid'] = $pid;
  }
  $form['public_domain'] = array(
    '#type' => 'checkbox',
    '#title' => t('This item is in the public domain'),
    '#description' => t('All uploaded items must be in the public domain'),
    '#default_value' => $pid ? TRUE : FALSE,
    '#required' => TRUE,
  );
  $form['author_first'] = array(
    '#type' => 'textfield',
    '#title' => t('Creator First Name'),
    '#description' => t("Creator first name (Author, painter, etc)"),
    '#default_value' => isset($results['first_name']) ? $results['first_name'] : '',
  );
  $form['author_last'] = array(
    '#type' => 'textfield',
    '#title' => t('Creator Last Name'),
    '#description' => t('Creator last name (Author, painter, etc)'),
    '#default_value' => isset($results['last_name']) ? $results['last_name'] : '',
    '#required' => TRUE,
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title of work'),
    '#default_value' => isset($results['title']) ? $results['title'][0] : '',
    '#required' => TRUE,
  );
  $form['dateIssued'] = array(
    '#type' => 'date_popup',
    '#title' => t('First publication'),
    '#date_format' => 'Y/m/d',
    '#default_value' => isset($results['date']) ? $results['date'][0] : '',
  );
  $form['country'] = array(
    '#type' => 'textfield',
    '#title' => t('Country'),
    '#description' => t('Country of first publication'),
    '#default_value' => isset($results['country']) ? $results['country'][0] : '',
  );
  $form['publisher'] = array(
    '#type' => 'textfield',
    '#title' => t('Publisher'),
    '#default_value' => isset($results['publisher']) ? $results['publisher'][0] : '',
  );
  $form['genre'] = array(
    '#type' => 'textfield',
    "#title" => t('Genre'),
    '#description' => t('(Poem, novel,manifesto, etc.)'),
    '#default_value' => isset($results['genre']) ? $results['genre'][0] : '',
  );
  $form['language'] = array(
    '#type' => 'textfield',
    "#title" => t('Language'),
    '#description' => t('Publication language'),
    '#default_value' => isset($results['language']) ? $results['language'][0] : 'English',
  );
  $form['manifestation'] = array(
    '#type' => 'textfield',
    '#title' => t('Manifestation'),
    '#description' => t('(periodical, book, broadside, etc.)'),
    '#default_value' => isset($results['manifestation']) ? $results['manifestation'][0] : '',
  );
  if (!$pid) {
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
      '#required' => TRUE,
      '#description' => t('Select file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $allowed)),
      '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => array($allowed),
        // Assume it's specified in MB.
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    );
  }
  $message = $pid ? t("Update Metadata") : t("Ingest Digital Asset");
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $message,
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
  //Edit mode.
  if (isset($form_state['pid'])) {
    $pid = $form_state['pid'];
    $object = islandora_object_load($form_state['pid']);
    $new_mods = coursepack_build_mods($form_state['values']);
    $object['MODS']->content = coursepack_build_mods($form_state['values']);
    drupal_set_message(t("Metadata updated"));
    $form_state['redirect'] = "islandora/object/$pid";
    return;
  }
  // Create new.
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
  add_manifestation($xml, $inputs['manifestation']);
  add_genre($xml, $inputs['genre']);
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

/**
 * Adds manifestation to document.
 *
 * @param DomDocument $xml
 *   DomDocument to which values are added
 * @param string $manifestation
 *   Manifestation to be added
 */
function add_manifestation($xml, $manifestation) {
  $mods_uri = "http://www.loc.gov/mods/v3";
  $root = $xml->documentElement;
  $physicalDescription_node = $xml->createElementNS($mods_uri, 'mods:physicalDescription');
  $root->appendChild($physicalDescription_node);
  $note_node = $xml->createElementNS($mods_uri, 'mods:note', $manifestation);
  $note_node->setAttribute('type', 'manifestation');
  $physicalDescription_node->appendChild($note_node);
}

function add_genre($xml, $genre) {
  $mods_uri = "http://www.loc.gov/mods/v3";
  $root = $xml->documentElement;
  $genre_node = $xml->createElementNS($mods_uri, 'mods:genre', $genre);
  $genre_node->setAttribute('authority', 'marcgt');
  $root->appendChild($genre_node);
}

/**
 * Retrieves fields to display in the Edit mode.
 *
 * @param AbstractObject $object
 *   The object to fetch the information from.
 *
 * @return array
 *   Any associative array where the key is the label and the second the
 *   fields value(s).
 */
function agile_coursepack_info_fields(AbstractObject $object) {
  if (!(isset($object['MODS']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['MODS']))) {
    return array();
  }
  $fields = array(
    'title' => '//mods:title',
    'date' => '//mods:dateCreated | //mods:dateIssued',
    'name' => array('//mods:name', 'mods:namePart'),
    'publisher' => '//mods:publisher',
    'genre' => '//mods:genre',
    'country' => '//mods:placeTerm',
    'manifestation' => '// mods:note[@type="manifestation"] ',
    'language' => '//mods:languageTerm',
  );
  $content = $object['MODS']->content;
  $xml = simplexml_load_string($object['MODS']->content);
  $xml->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');
  $to_string = function($o) {
    return (string) $o;
  };
  // Query each context node with the given xpath.
  $query_each_context = function (array $contexts, $xpath) {
    $query = function(&$context, $key, $xpath) {
      $context->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');
      $context = $context->xpath($xpath);
    };
    // Map values to each context node.
    array_walk($contexts, $query, $xpath);
    // Flatten to an array of values.
    return count($contexts) ? call_user_func_array('array_merge', $contexts) : array();
  };
  // Query the given xpath. If the xpath is any array the first value is the
  // xpath for the context node(s) and the second the path to the value fields.
  $query = function(SimpleXMLElement $xml, $xpath) use(&$query_each_context) {
    return is_string($xpath) ? $xml->xpath($xpath) : $query_each_context($xml->xpath($xpath[0]), $xpath[1]);
  };
  foreach ($fields as $label => $xpath) {
    $results = $query($xml, $xpath);
    // Remove empty values.
    $fields[$label] = array_filter(array_map($to_string, $results));
  }
  // Remove empty fields.
  return array_filter($fields);
}
