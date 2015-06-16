<?php

class GLField {

  public $type;//String for field/metatag/something else

  public $contentType;//Content Type for this Field object
  public $parentFCName;//Parent FC Name for recursive FCs
  public $bundle;//Bundle name for this entity
  public $entityId; //String e.g. entity id for field collection
  public $entityType; //String e.g. node or field_collection_item
  public $fieldName; //String e.g. field_case_study_title
  public $fieldLabel; //String e.g. Title
  public $translatedContent;//String
  public $delta;
  public $format;
  public $langcode;

}
