<?php

require_once TPT_ROOT . '/' . ('gl_ws/client/SessionService2.php');
require_once TPT_ROOT . '/' . ('gl_ws/client/ProjectService2.php');
require_once TPT_ROOT . '/' . ('gl_ws/client/DocumentService2.php');
require_once TPT_ROOT . '/' . ('gl_ws/client/SubmissionService2.php');

if (filter_input(INPUT_SERVER, 'HTTP_X_FORWARDED_HOST')) {
  define('GL_WSDL_PATH', 'http://' . filter_input(INPUT_SERVER, 'HTTP_X_FORWARDED_HOST')
          . base_path() . drupal_get_path('module', 'transperfect') . '/gl_ws/wsdl/');
}
else {
  define('GL_WSDL_PATH', 'http://' . filter_input(INPUT_SERVER, 'HTTP_HOST')
          . base_path() . drupal_get_path('module', 'transperfect') . '/gl_ws/wsdl/');
}

function get_user_PD_projects($pdObject) {
  $username = $pdObject->username;
  $password = $pdObject->password;
  $url = $pdObject->url;

  //Create instances for each ws service stub class
  $sessionService = new SessionService2(GL_WSDL_PATH . 'SessionService2.wsdl', array(
              'location' => $url . "/services/SessionService2"
          ));
  $projectService = new ProjectService2(GL_WSDL_PATH . 'ProjectService2.wsdl', array(
              'location' => $url . "/services/ProjectService2"
          ));

  $token = @Login($sessionService, $username, $password);
  $projects = GetUserProjects($projectService, $token);

  $proj_arr = array();
  foreach ($projects as $project) {
    $short_code = $project->projectInfo->shortCode;
    $proj_arr[$short_code] = $project->projectInfo->name;
  }

  return $proj_arr;
}

function test_ProjectDirector_connectivity($pdObject) {
  $username = $pdObject->username;
  $password = $pdObject->password;
  $url = $pdObject->url;
  $projectShortCode = $pdObject->projectShortCode;
  $proj_arr = array();
  if ($projectShortCode != '') {
    $arr = explode(',', $projectShortCode);
    foreach ($arr as $value) {
      $proj_arr[$value] = $value;
    }
  }

  //Create instances for each ws service stub class
  $sessionService = new SessionService2(GL_WSDL_PATH . 'SessionService2.wsdl', array(
              'location' => $url . "/services/SessionService2"
          ));
  $projectService = new ProjectService2(GL_WSDL_PATH . 'ProjectService2.wsdl', array(
              'location' => $url . "/services/ProjectService2"
          ));


  $success = FALSE;
  //Open a session on the server
  $token = @Login($sessionService, $username, $password);
  //Locate project by short code
  foreach ($proj_arr as $proj_code) {
    try {
      $project = @FindProjectByShortCode($projectService, $proj_code, $token);
    }
    catch (SoapFault $se) {
      gl_log(TPT_OBJECT_TYPE_GLOBALLINK, TPT_LOGGING_SEVERITY_ERROR, "SOAP Exception in Test Connection - Code[$se->faultcode], Message[$se->faultstring]");
      return 'Connection Failed - Invalid Project Code: ' . $proj_code;
    }
    catch (Exception $e) {
      gl_log(TPT_OBJECT_TYPE_GLOBALLINK, TPT_LOGGING_SEVERITY_ERROR, "Exception in Test Connection - getReadyTranslationsDetailsFromPD - File[" . $e->getFile() . "], Line[" . $e->getLine() . "], Code[" . $e->getCode() . "], Message[" . $e->getMessage() . "]");
      return 'Connection Failed - Invalid Project Code: ' . $proj_code;
    }

    if (isset($project->ticket)) {
      if (isset($project->fileFormatProfiles) && is_array($project->fileFormatProfiles)) {
        foreach ($project->fileFormatProfiles as $file_format) {
          if ($pdObject->classifier == $file_format->profileName) {
            $success = TRUE;
            break;
          }
        }
      }
      elseif (isset($project->fileFormatProfiles)) {
        if ($pdObject->classifier == $project->fileFormatProfiles->profileName) {
          $success = TRUE;
        }
      }
    }

    if (!$success) {
      return 'Connection Failed - Invalid Classifier.';
    }
    else {
      $success = FALSE;
    }
  }

  return TRUE;
}

function cancel_ProjectDirector_submission($pdObject, &$globalLink) {
  $username = $pdObject->username;
  $password = $pdObject->password;
  $url = $pdObject->url;

  //Create instances for each ws service stub class
  $sessionService = new SessionService2(GL_WSDL_PATH . 'SessionService2.wsdl', array(
              'location' => $url . "/services/SessionService2"
          ));
  $submissionService = new SubmissionService2(GL_WSDL_PATH . 'SubmissionService2.wsdl', array(
              'location' => $url . "/services/SubmissionService2"
          ));

  $token = @Login($sessionService, $username, $password);

  $cancelSubmissionRequest = new cancelSubmission();

  $cancelSubmissionRequest->userId = $token;
  $cancelSubmissionRequest->submissionId = $globalLink->submissionTicket;

  $cancelSubmissionResponse = $submissionService->cancelSubmission($cancelSubmissionRequest);
  $globalLink->cancelled = TRUE;

  return $cancelSubmissionResponse->return;
}

function cancel_select_documents($pdObject, &$globalLink_arr) {
  $username = $pdObject->username;
  $password = $pdObject->password;
  $url = $pdObject->url;

  //Create instances for each ws service stub class
  $sessionService = new SessionService2(GL_WSDL_PATH . 'SessionService2.wsdl', array(
              'location' => $url . "/services/SessionService2"
          ));
  $targetService = new TargetService2(GL_WSDL_PATH . 'TargetService2.wsdl', array(
              'location' => $url . "/services/TargetService2"
          ));

  $token = @Login($sessionService, $username, $password);

  foreach ($globalLink_arr as $globalLink) {
    $cancelTargetSubmission = new cancelTargetByDocumentId();
    $documentTicket = new DocumentTicket();
    $documentTicket->submissionTicket = $globalLink->submissionTicket;
    $documentTicket->ticketId = $globalLink->documentTicket;
    $cancelTargetSubmission->documentId = $documentTicket;
    $cancelTargetSubmission->targetLocale = $globalLink->targetLocale;
    $cancelTargetSubmission->userId = $token;

    $targetService->cancelTargetByDocumentId($cancelTargetSubmission);
    $globalLink->cancelled = TRUE;
  }
}

function GetUserProjects($projectService, $token) {
  $getUserProjects = new getUserProjects();

  $getUserProjects->isSubProjectIncluded = FALSE;
  $getUserProjects->userId = $token;

  $getUserProjectsResponse = $projectService->getUserProjects($getUserProjects);
  $projects = $getUserProjectsResponse->return;

  if (!is_array($projects)) {
    $arr = array($projects);
    return $arr;
  }

  return $projects;
}

function FindProjectByShortCode($projectService, $projectShortCode, $token) {
  $findProjectByShortCode = new findProjectByShortCode();

  $findProjectByShortCode->projectShortCode = $projectShortCode;
  $findProjectByShortCode->userId = $token;

  $findProjectByShortCodeResponse = $projectService->findProjectByShortCode($findProjectByShortCode);

  $project = $findProjectByShortCodeResponse->return;

  return $project;
}

function Login($sessionService, $username, $password) {

  $login = new login();

  $login->username = $username;
  $login->password = $password;

  $loginResponse = $sessionService->login($login);

  $token = $loginResponse->return;

  return $token;
}

function get_status($pdObject, $submissionTicket) {
  $username = $pdObject->username;
  $password = $pdObject->password;
  $url = $pdObject->url;

  $sessionService = new SessionService2(GL_WSDL_PATH . 'SessionService2.wsdl', array(
              'location' => $url . "/services/SessionService2"
          ));
  $submissionService = new SubmissionService2(GL_WSDL_PATH . 'SubmissionService2.wsdl', array(
              'location' => $url . "/services/SubmissionService2"
          ));

  $token = Login($sessionService, $username, $password);

  $ticketRequest = new findByTicket();
  $ticketRequest->userId = $token;
  $ticketRequest->ticket = $submissionTicket;

  $findByTicketResponse = $submissionService->findByTicket($ticketRequest);

  $result = $findByTicketResponse->return;
  $status = '';

  if (isset($result->status))
    $status = $result->status->name;

  return $status;
}