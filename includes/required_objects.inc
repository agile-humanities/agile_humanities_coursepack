<?php

function agile_coursepack_get_required_objects($connection) {
  $module_path = drupal_get_path('module', 'agile_coursepack');
  // Document Content Model.
  $coursepack_content_model = $connection->repository->constructObject('islandora:coursepackCModel');
  $coursepack_content_model->owner = 'fedoraAdmin';
  $coursepack_content_model->label = 'Islandora Coursepack Content Model';
  $coursepack_content_model->models = 'fedora-system:ContentModel-3.0';
  // DS-COMPOSITE-MODEL Datastream.
  $datastream = $coursepack_content_model->constructDatastream('DS-COMPOSITE-MODEL', 'X');
  $datastream->label = 'DS-COMPOSITE-MODEL';
  $datastream->mimetype = 'text/xml';
  $datastream->setContentFromFile("$module_path/xml/islandora_coursepack_ds_composite_model.xml", FALSE);
  $coursepack_content_model->ingestDatastream($datastream);
  // Coursepack Collection.
  $coursepack_collection = $connection->repository->constructObject('islandora:coursepacks');
  $coursepack_collection->owner = 'fedoraAdmin';
  $coursepack_collection->label = 'Coursepacks';
  $coursepack_collection->models = 'islandora:collectionCModel';
  $coursepack_collection->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', 'islandora:root');
  // Collection Policy Datastream.
  $datastream = $coursepack_collection->constructDatastream('COLLECTION_POLICY', 'X');
  $datastream->label = 'Collection policy';
  $datastream->mimetype = 'text/xml';
  $datastream->setContentFromFile("$module_path/xml/agile_coursepack_collection_policy.xml", FALSE);
  $coursepack_collection->ingestDatastream($datastream);
  // TN Datastream.
  $datastream = $coursepack_collection->constructDatastream('TN', 'M');
  $datastream->label = 'Thumbnail';
  $datastream->mimetype = 'image/png';
  $datastream->setContentFromFile("$module_path/images/folder.png", FALSE);
  $coursepack_collection->ingestDatastream($datastream);
  return array(
    'agile_coursepack' => array(
      'title' => 'Agile Coursepack',
      'objects' => array(
        $coursepack_collection,
        $coursepack_content_model,
      ),
    ),
  );
}
