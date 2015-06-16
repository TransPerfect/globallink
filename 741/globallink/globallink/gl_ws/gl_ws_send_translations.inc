<?php

require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_common.inc');
require_once TPT_ROOT . '/' . ('gl_ws/client/SessionService2.php');
require_once TPT_ROOT . '/' . ('gl_ws/client/ProjectService2.php');
require_once TPT_ROOT . '/' . ('gl_ws/client/DocumentService2.php');
require_once TPT_ROOT . '/' . ('gl_ws/client/SubmissionService2.php');

function sendDocumentsForTranslationToPD(&$globalLink_arr, $pdObject, $project_code, $submitter) {
  //Define communication constants
  $username = $pdObject->username;
  $password = $pdObject->password;
  $url = $pdObject->url;
  $projectShortCode = $project_code;

  //Create instances for each ws service stub class
  $sessionService = new SessionService2(GL_WSDL_PATH . 'SessionService2.wsdl', array(
              'location' => $url . "/services/SessionService2"
          ));
  $projectService = new ProjectService2(GL_WSDL_PATH . 'ProjectService2.wsdl', array(
              'location' => $url . "/services/ProjectService2"
          ));
  $documentService = new DocumentService2(GL_WSDL_PATH . 'DocumentService2.wsdl', array(
              'location' => $url . "/services/DocumentService2"
          ));
  $submissionService = new SubmissionService2(GL_WSDL_PATH . 'SubmissionService2.wsdl', array(
              'location' => $url . "/services/SubmissionService2"
          ));

  $GLOBALS['g_submissionTicket'] = '';
  $GLOBALS['g_document_count'] = 0;
  $clientIdentifier = $projectShortCode . REQUEST_TIME;

  $token = @Login($sessionService, $username, $password);
  $project = @FindProjectByShortCode($projectService, $projectShortCode, $token);
  $submissionInfo = createSubmissionInfo($project->ticket, $clientIdentifier, $submitter);

  $count = 0;
  foreach ($globalLink_arr as $globalLink) {
    $count++;
    $submissionInfo->name = $globalLink->submissionName;
    $dateRequested = new Date();
    $dateRequested->date = $globalLink->dueDate;
    $dateRequested->critical = false;
    $submissionInfo->dateRequested = $dateRequested;
    $resourceInfo = createResourceInfo($clientIdentifier, $pdObject, $globalLink);
    $documentInfo = createDocumentInfo($project->ticket, $globalLink, $clientIdentifier);
    if (!is_null($GLOBALS['g_submissionTicket']) && $GLOBALS['g_submissionTicket'] != '') {
      $documentInfo->submissionTicket = $GLOBALS['g_submissionTicket'];
    }
    //Submit a document for the given project
    @submitDocumentWithBinaryResource($documentService, $resourceInfo, $documentInfo, $globalLink, $token);
    $GLOBALS['g_submissionTicket'] = $globalLink->submissionTicket;
    $GLOBALS['g_document_count'] = $count;
  }

  @startSubmission($submissionService, $submissionInfo, $token);
}

function submitDocumentWithBinaryResource($documentService, $resourceInfo, $documentInfo, &$globalLink, $token) {

  $submitDocumentWithBinaryResourceRequest = new submitDocumentWithBinaryResource;

  $submitDocumentWithBinaryResourceRequest->documentInfo = $documentInfo;
  $submitDocumentWithBinaryResourceRequest->resourceInfo = $resourceInfo;
  $submitDocumentWithBinaryResourceRequest->data = $globalLink->sourceXML;
  $submitDocumentWithBinaryResourceRequest->userId = $token;

  $submitDocumentWithBinaryResourceResponse = $documentService->submitDocumentWithBinaryResource($submitDocumentWithBinaryResourceRequest);

  $globalLink->submissionTicket = $submitDocumentWithBinaryResourceResponse->return->submissionTicket;
  $globalLink->documentTicket = $submitDocumentWithBinaryResourceResponse->return->ticketId;
}

function startSubmission($submissionService, $submissionInfo, $token) {
  $startSubmissionRequest = new startSubmission;

  $startSubmissionRequest->submissionId = $GLOBALS['g_submissionTicket'];
  $startSubmissionRequest->submissionInfo = $submissionInfo;
  $startSubmissionRequest->userId = $token;

  $startSubmissionResponse = $submissionService->startSubmission($startSubmissionRequest);

  return $startSubmissionResponse->return;
}

function createSubmissionInfo($projectTicket, $clientIdentifier, $submitter) {
  $metadata = new Metadata;
  $metadata->key = 'drupalUser';
  $metadata->value = $submitter;

  $submissionInfo = new SubmissionInfo();
  $submissionInfo->clientIdentifier = $clientIdentifier;
  $submissionInfo->projectTicket = $projectTicket;
  $submissionInfo->metadata = $metadata;
  return $submissionInfo;
}

function createDocumentInfo($projectTicket, &$globalLink, $clientIdentifier) {
  $md_arr = array();

  $metadata = new Metadata;
  $metadata->key = 'type';
  $metadata->value = $globalLink->metadata;
  array_push($md_arr, $metadata);

  $metadata = new Metadata;
  $metadata->key = 'type_id';
  if($globalLink->metadata != 'Node' && $globalLink->metadata != 'Entity') {
    $metadata->value = $globalLink->otherObjectId;
  }
  else {
    $metadata->value = $globalLink->nid;
  }
  array_push($md_arr, $metadata);

  $documentInfo = new DocumentInfo();

  $documentInfo->metadata = $md_arr;
  $documentInfo->projectTicket = $projectTicket;
  $documentInfo->sourceLocale = $globalLink->sourceLocale;
  $documentInfo->name = $globalLink->sourceFileName;
  $documentInfo->clientIdentifier = $clientIdentifier;
  $dateRequested = new Date();
  $dateRequested->date = $globalLink->dueDate;
  $dateRequested->critical = false;
  $documentInfo->dateRequested = $dateRequested;
  $documentInfo->instructions = $globalLink->submissionInstructions;

  $targetInfos = array();
  $count = 0;
  foreach ($globalLink->targetLocale as $key => $value) {
    $targetInfos[$count] = createTargetInfo($value, $globalLink);
    $count++;
  }

  $documentInfo->targetInfos = $targetInfos;
  return $documentInfo;
}

function createTargetInfo($locale, &$globalLink) {
  $targetInfo = new TargetInfo();

  $targetInfo->targetLocale = $locale;
  $dateRequested = new Date();
  $dateRequested->date = $globalLink->dueDate;
  $dateRequested->critical = false;
  $targetInfo->dateRequested = $dateRequested;
  $targetInfo->requestedDueDate = $globalLink->dueDate;
  $priority = new Priority();
  $priority->value = 1;
  $targetInfo->priority = $priority;
  $targetInfo->encoding = "UTF-8";

  return $targetInfo;
}

function createResourceInfo($clientIdentifier, &$pdObject, $globalLink) {
  $resourceInfo = new ResourceInfo();

  $resourceType = new ResourceType();
  $resourceType->value = 0;

  $resourceInfo->type = $resourceType;
  $resourceInfo->mimeType = "text/xml";
  $resourceInfo->classifier = $pdObject->classifier;
  $resourceInfo->name = $globalLink->sourceFileName;
  $resourceInfo->encoding = "UTF-8";
  $resourceInfo->size = strlen($globalLink->sourceXML);
  $resourceInfo->clientIdentifier = $clientIdentifier;
  $resourceInfo->resourceInfoId = $GLOBALS['g_document_count'];

  return $resourceInfo;
}