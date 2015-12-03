<?php

/**
 * @file
 * Implements a simple class for working with Coursepack data.
 */

/**
 * Coursepack Data Class
 */
class CPData {

  public $cp = array(
    'cp:name' => array(),
    'cp:number' => array(),
    'cp:date' => array(),
    'cp:year' => array(),
    'cp:term' => array(),
    'cp:instructorfirst' => array(),
    'cp:instructorlast' => array(),
    'cp:instructortitle' => array(),
    'cp:department' => array(),
    'cp:institution' => array(),
  );
  public $owner;

  /**
   * Constructor.
   *
   * @param string $cp_xml
   *   The Coursepack Data XML.
   */
  public function __construct($cp_xml = NULL) {
    if (!empty($cp_xml)) {
      $this->cp = self::importFromXMLString($cp_xml);
    }
  }

  /**
   * Add an element.
   *
   * @param string $element_name
   *   The name of the element to add.
   * @param string $value
   *   The value of the element to add.
   */
  public function addElement($element_name, $value) {
    if (is_string($value) && is_array($this->cp[$element_name])) {
      $this->cp[$element_name][] = $value;
    }
  }

  /**
   * Replace the given Courespack element with the given values.
   *
   * @param string $element_name
   *   The name of the elements to set.
   * @param mixed $values
   *   The values of the set the elements too.
   */
  public function setElement($element_name, $values) {
    if (is_array($values)) {
      $this->cp[$element_name] = $values;
    }
    elseif (is_string($values)) {
      $this->cp[$element_name] = array($values);
    }
  }

  /**
   * Serialize this object as an XML string.
   *
   * @return string
   *   The serialized XML.
   */
  public function asXML() {
    $cp_xml = new DomDocument();
    $cp_xml->formatOutput = true;
    $cp_root = $cp_xml->createElementNS('http://agilehumanities.ca/cp', 'cp:coursepack');
    $cp_root->setAttribute('xmlns:cp', 'http://agilehumanities.ca/cp');
    $this->cp = array_filter($this->cp);
    foreach ($this->cp as $cp_element => $values) {
      if (is_array($values) && !empty($values)) {
        foreach ($values as $value) {
          $new_item = $cp_xml->createElement($cp_element, $value);
          $cp_root->appendChild($new_item);
        }
      }
      else {
        $new_item = $cp_xml->createElement($cp_element);
        $cp_root->appendChild($new_item);
      }
    }
    $cp_xml->appendChild($cp_root);
    return $cp_xml->saveXML();
  }

  /**
   * Serializes this object as an array.
   *
   * @return array
   *   The serialized object.
   */
  public function asArray() {
    $cp_array = array();
    foreach ($this as $element) {
      if (!empty($element)) {
        foreach ($element as $field => $values) {
          // Split value if the result value is an array.
          if (is_array($values)) {
            $value = '';
            $i = 0;
            foreach ($values as $piece) {
              if (!empty($piece)) {
                if ($i++) {
                  $value .= ", ";
                }
                $value .= $piece;
              }
            }
          }
          else {
            $value = $values;
          }
          $cp_label = explode(':', $field);
          $element_label = drupal_ucfirst($cp_label[1]);
          $i18n_object_id = drupal_strtolower($element_label);
          $cp_array[$field]['label'] = function_exists('i18n_string') ?
            i18n_string("islandora:cp:{$i18n_object_id}:label", $element_label) :
            $element_label;
          $cp_array[$field]['value'] = filter_xss($value);
          $cp_array[$field]['class'] = drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $field));
          $cp_array[$field]['dcterms'] = preg_replace('/^dc/', 'dcterms', $field);
        }
      }
    }
    return $cp_array;
  }

  /**
   * Creates a new instance of the class by parsing cp_xml.
   *
   * @param string $cp_xml
   *   Dublin Core XML.
   *
   * @return CPData
   *   The instantiated object.
   */
  public static function importFromXMLString($cp_xml) {
    $cp_doc = new DomDocument();
    if (!empty($cp_xml) && $cp_doc->loadXML($cp_xml)) {
      $cp_root = $cp_doc->getElementsByTagNameNS('http://agilehumanities.ca/ca', '*');
      $new_cp = new CPData();
      foreach ($cp_root as $child) {
        if (isset($new_cp->cp[$child->nodeName])) {
          array_push($new_cp->cp[$child->nodeName], $child->nodeValue);
        }
      }
      return $new_cp;
    }
    return NULL;
  }

}
