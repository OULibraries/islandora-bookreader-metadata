<?php

/**
 * @file
 * Handles all the callbacks.
 *
 * Currently there are three callbacks:
 * - Dimensions: Returns the page dimensions in JSON.
 * - Text: Returns the OCR'd page text as an html string.
 * - Search: Returns solr search results and word coordinates as JSON.
 */

/**
 * Fetches page metadata for use in the viewer such as width and height.
 *
 * Prints a JSON object repersenting the metadata.
 * This function exits.
 *
 * @param AbstractObject $object
 *   The page object to fetch the metadata from.
 */
function islandora_internet_archive_bookreader_dimensions_callback(AbstractObject $object) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $dimensions = islandora_paged_content_get_page_dimensions($object);
  drupal_json_output($dimensions);
  drupal_exit();
}

/**
 * Prints the given page objects OCR datastream.
 *
 * This function exits.
 *
 * @param AbstractObject $object
 *   The page object whose OCR data-stream will be used for display.
 */
function islandora_internet_archive_bookreader_text_callback(AbstractObject $object) {
  $ocr_exists = isset($object['OCR']);
  $accessible_ocr = isset($object['OCR']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['OCR']);
  echo $ocr_exists ? (
    $accessible_ocr ?
      htmlspecialchars($object['OCR']->content) :
      t('You do not have permission to view the OCR for this page.')) :
    t('Sorry, but there is no OCR available for the this page.');

  drupal_exit();
}

/**
 * Performs a solr search for the given term within the given book.
 *
 * Prints a JSON object repersenting the results.
 * This function exits.
 *
 * @param string $object_id
 *   The PID of the book object.
 * @param string $term
 *   The text to search for within the OCR'ed text.
 */
function islandora_internet_archive_bookreader_search_callback($object_id, $term) {
  module_load_include('inc', 'islandora_paged_content', 'includes/highlighting');
  module_load_include('inc', 'islandora_internet_archive_bookreader', 'includes/utilities');
  if (!module_exists('islandora_solr')) {
    drupal_exit();
  }

  $results = islandora_paged_content_perform_solr_highlighting_query($term, array(
    'fq' => array(format_string('!field:("info:fedora/!value" OR "!value")', array(
      '!field' => variable_get('islandora_internet_archive_bookreader_ocr_filter_field', 'RELS_EXT_isMemberOf_uri_ms'),
      '!value' => $object_id,
    ))),
    'hl.tag.pre' => '{{{',
    'hl.tag.post' => '}}}',
    'defType' => 'dismax',
  ));
  $results = islandora_internet_archive_bookreader_remap_highlighting_for_bookreader_results($results);
  drupal_json_output($results);
  drupal_exit();
}
