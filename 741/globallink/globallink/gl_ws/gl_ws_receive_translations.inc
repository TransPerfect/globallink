<?php

require_once TPT_ROOT . '/' . ('gl_ws/gl_ws_common.inc');
require_once TPT_ROOT . '/' . ('gl_ws/client/SessionService2.php');
require_once TPT_ROOT . '/' . ('gl_ws/client/ProjectService2.php');
require_once TPT_ROOT . '/' . ('gl_ws/client/TargetService2.php');

function getReadyTranslationsDetailsFromPD($pdObject, $type = 'node') {
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

  $sessionService = new SessionService2(GL_WSDL_PATH . 'SessionService2.wsdl', array(
              'location' => $url . "/services/SessionService2"
          ));
  $projectService = new ProjectService2(GL_WSDL_PATH . 'ProjectService2.wsdl', array(
              'location' => $url . "/services/ProjectService2"
          ));
  $targetService = new TargetService2(GL_WSDL_PATH . 'TargetService2.wsdl', array(
              'location' => $url . "/services/TargetService2"
          ));

  $globalLink_arr = array();

  $token = @Login($sessionService, $username, $password);
  $projects = GetUserProjects($projectService, $token);
  $proj_ticket_arr = array();
  foreach ($projects as $project) {
    $short_code = $project->projectInfo->shortCode;
    foreach ($proj_arr as $conf_proj) {
      if ($conf_proj == $short_code) {
        $proj_ticket_arr[] = $project->ticket;
      }
    }
  }

  if (count($proj_ticket_arr) > 0) {
    $targets = @getCompletedTargetsByProjects($targetService, $proj_ticket_arr, $token, $pdObject);
    $count = 1;
    if (is_array($targets)) {
      foreach ($targets as $target) {
        if (!is_null($target->ticket) && $target->ticket != '') {
          $sourceFile = $target->document->documentInfo->name;
          $m_arr = $target->document->documentInfo->metadata;
          $sourceType = '';
          $type_id = '';
          if (is_array($m_arr)) {
            foreach ($m_arr as $m) {
              if ($m->key == 'type') {
                $sourceType = $m->value;
              }
              else {
                $type_id = $m->value;
              }
            }
          }
          else {
            $sourceType = $target->document->documentInfo->metadata->value;
          }
          if ($type == 'entity') {
            if ($sourceType != 'Entity') {
              continue;
            }
          }
          elseif ($type == 'menu') {
            if ($sourceType != 'Menu') {
              continue;
            }
          }
          elseif ($type == 'block') {
            if ($sourceType != 'Block') {
              continue;
            }
          }
          elseif ($type == 'taxonomy') {
            if ($sourceType != 'Taxonomy') {
              continue;
            }
          }
          elseif ($type == 'interface') {
            if ($sourceType != 'Interface') {
              continue;
            }
          }
          elseif ($type == 'webform') {
            if ($sourceType != 'Webform') {
              continue;
            }
          }
          elseif ($type == 'fieldable_panels') {
            if ($sourceType != 'Fieldable_Panels_Panes') {
              continue;
            }
          }
          else {
            if ($sourceType == 'Entity' || $sourceType == 'Menu' || $sourceType == 'Block' || $sourceType == 'Taxonomy' || $sourceType == 'Interface' || $sourceType == 'Webform' || $sourceType == 'Fieldable_Panels_Panes') {
              continue;
            }
          }

          $globalLink = new GlobalLink();
          $globalLink->submissionTicket = $target->document->documentGroup->submission->ticket;
          $globalLink->submissionName = $target->document->documentGroup->submission->submissionInfo->name;
          $globalLink->documentTicket = $target->document->ticket;
          $globalLink->sourceFileName = $sourceFile;
          $globalLink->targetTicket = $target->ticket;
          $globalLink->sourceLocale = str_replace("-", "_", $target->sourceLanguage->locale);
          $globalLink->targetLocale = str_replace("-", "_", $target->targetLanguage->locale);
          $globalLink->type = $sourceType;
          if ($sourceType != 'Node' && $sourceType != 'Entity') {
            $globalLink->otherObjectId = $type_id;
          }
          else {
            $globalLink->nid = $type_id;
          }
          $globalLink_arr[$count] = $globalLink;
          $count++;
        }
      }
    }
    elseif (!is_null($targets)) {
      $target = $targets;
      if (!is_null($target->ticket) && $target->ticket != '') {
        $sourceFile = $target->document->documentInfo->name;
        $m_arr = $target->document->documentInfo->metadata;
        $sourceType = '';
        $type_id = '';
        if (is_array($m_arr)) {
          foreach ($m_arr as $m) {
            if ($m->key == 'type') {
              $sourceType = $m->value;
            }
            else {
              $type_id = $m->value;
            }
          }
        }
        else {
          $sourceType = $target->document->documentInfo->metadata->value;
        }
        if ($type == 'entity') {
          if ($sourceType != 'Entity') {
            return $globalLink_arr;
          }
        }
        elseif ($type == 'menu') {
          if ($sourceType != 'Menu') {
            return $globalLink_arr;
          }
        }
        elseif ($type == 'block') {
          if ($sourceType != 'Block') {
            return $globalLink_arr;
          }
        }
        elseif ($type == 'taxonomy') {
          if ($sourceType != 'Taxonomy') {
            return $globalLink_arr;
          }
        }
        elseif ($type == 'interface') {
          if ($sourceType != 'Interface') {
            return $globalLink_arr;
          }
        }
        elseif ($type == 'webform') {
          if ($sourceType != 'Webform') {
            return $globalLink_arr;
          }
        }
        elseif ($type == 'fieldable_panels') {
          if ($sourceType != 'Fieldable_Panels_Panes') {
            return $globalLink_arr;
          }
        }
        else {
          if ($sourceType == 'Entity' || $sourceType == 'Menu' || $sourceType == 'Block' || $sourceType == 'Taxonomy' || $sourceType == 'Interface' || $sourceType == 'Webform' || $sourceType == 'Fieldable_Panels_Panes') {
            return $globalLink_arr;
          }
        }

        $globalLink = new GlobalLink();
        $globalLink->submissionTicket = $target->document->documentGroup->submission->ticket;
        $globalLink->submissionName = $target->document->documentGroup->submission->submissionInfo->name;
        $globalLink->documentTicket = $target->document->ticket;
        $globalLink->sourceFileName = $sourceFile;
        $globalLink->targetTicket = $target->ticket;
        $globalLink->sourceLocale = str_replace("-", "_", $target->sourceLanguage->locale);
        $globalLink->targetLocale = str_replace("-", "_", $target->targetLanguage->locale);
        $globalLink->type = $sourceType;
        if ($sourceType != 'Node' && $sourceType != 'Entity') {
          $globalLink->otherObjectId = $type_id;
        }
        else {
          $globalLink->nid = $type_id;
        }
        $globalLink_arr[$count] = $globalLink;
      }
    }
  }
  return $globalLink_arr;
}

function getAllReadyTranslationsDetailsFromPD($pdObject) {
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

  $sessionService = new SessionService2(GL_WSDL_PATH . 'SessionService2.wsdl', array(
              'location' => $url . "/services/SessionService2"
          ));
  $projectService = new ProjectService2(GL_WSDL_PATH . 'ProjectService2.wsdl', array(
              'location' => $url . "/services/ProjectService2"
          ));
  $targetService = new TargetService2(GL_WSDL_PATH . 'TargetService2.wsdl', array(
              'location' => $url . "/services/TargetService2"
          ));

  $globalLink_arr = array();

  $token = @Login($sessionService, $username, $password);
  $projects = GetUserProjects($projectService, $token);
  $proj_ticket_arr = array();
  foreach ($projects as $project) {
    $short_code = $project->projectInfo->shortCode;
    foreach ($proj_arr as $conf_proj) {
      if ($conf_proj == $short_code) {
        $proj_ticket_arr[] = $project->ticket;
      }
    }
  }

  if (count($proj_ticket_arr) > 0) {
    $targets = @getCompletedTargetsByProjects($targetService, $proj_ticket_arr, $token, $pdObject);
    $count = 1;
    if (is_array($targets)) {
      foreach ($targets as $target) {
        if (!is_null($target->ticket) && $target->ticket != '') {
          $sourceFile = $target->document->documentInfo->name;
          $m_arr = $target->document->documentInfo->metadata;
          $sourceType = '';
          $type_id = '';
          if (is_array($m_arr)) {
            foreach ($m_arr as $m) {
              if ($m->key == 'type') {
                $sourceType = $m->value;
              }
              else {
                $type_id = $m->value;
              }
            }
          }
          else {
            $sourceType = $target->document->documentInfo->metadata->value;
          }

          $globalLink = new GlobalLink();
          $globalLink->submissionTicket = $target->document->documentGroup->submission->ticket;
          $globalLink->submissionName = $target->document->documentGroup->submission->submissionInfo->name;
          $globalLink->documentTicket = $target->document->ticket;
          $globalLink->sourceFileName = $sourceFile;
          $globalLink->targetTicket = $target->ticket;
          $globalLink->sourceLocale = str_replace("-", "_", $target->sourceLanguage->locale);
          $globalLink->targetLocale = str_replace("-", "_", $target->targetLanguage->locale);
          $globalLink->type = $sourceType;
          if ($sourceType != 'Node') {
            $globalLink->otherObjectId = $type_id;
          }
          else {
            $globalLink->nid = $type_id;
          }

          $globalLink_arr[$count] = $globalLink;
          $count++;
        }
      }
    }
    elseif (!is_null($targets)) {
      $target = $targets;
      if (!is_null($target->ticket) && $target->ticket != '') {
        $sourceFile = $target->document->documentInfo->name;
        $m_arr = $target->document->documentInfo->metadata;
        $sourceType = '';
        $type_id = '';
        if (is_array($m_arr)) {
          foreach ($m_arr as $m) {
            if ($m->key == 'type') {
              $sourceType = $m->value;
            }
            else {
              $type_id = $m->value;
            }
          }
        }
        else {
          $sourceType = $target->document->documentInfo->metadata->value;
        }

        $globalLink = new GlobalLink();
        $globalLink->submissionTicket = $target->document->documentGroup->submission->ticket;
        $globalLink->submissionName = $target->document->documentGroup->submission->submissionInfo->name;
        $globalLink->documentTicket = $target->document->ticket;
        $globalLink->sourceFileName = $sourceFile;
        $globalLink->targetTicket = $target->ticket;
        $globalLink->sourceLocale = str_replace("-", "_", $target->sourceLanguage->locale);
        $globalLink->targetLocale = str_replace("-", "_", $target->targetLanguage->locale);
        $globalLink->type = $sourceType;
        if ($sourceType != 'Node') {
          $globalLink->otherObjectId = $type_id;
        }
        else {
          $globalLink->nid = $type_id;
        }

        $globalLink_arr[$count] = $globalLink;
      }
    }
  }
  return $globalLink_arr;
}

function getCompletedTargetsByProjects($targetService, $projectTickets, $token, $pdObject) {
  $getCompletedTargetsByProjectsRequest = new getCompletedTargetsByProjects();

  $getCompletedTargetsByProjectsRequest->projectTickets = $projectTickets;
  $getCompletedTargetsByProjectsRequest->maxResults = $pdObject->maxTargetCount;
  $getCompletedTargetsByProjectsRequest->userId = $token;

  $getCompletedTargetsByProjectsResponse = $targetService->getCompletedTargetsByProjects($getCompletedTargetsByProjectsRequest);

  return $getCompletedTargetsByProjectsResponse->return;
}

function getCompletedTargetsByDoc($pdObject, $ticket) {
  $username = $pdObject->username;
  $password = $pdObject->password;
  $url = $pdObject->url;

  $sessionService = new SessionService2(GL_WSDL_PATH . 'SessionService2.wsdl', array(
              'location' => $url . "/services/SessionService2"
          ));

  $targetService = new TargetService2(GL_WSDL_PATH . 'TargetService2.wsdl', array(
              'location' => $url . "/services/TargetService2"
          ));

  $token = Login($sessionService, $username, $password);

  $getCompletedTargetsByDocumentsRequest = new getCompletedTargetsByDocuments();

  $getCompletedTargetsByDocumentsRequest->documentTickets = array($ticket);
  $getCompletedTargetsByDocumentsRequest->maxResults = 99;
  $getCompletedTargetsByDocumentsRequest->userId = $token;

  $getCompletedTargetsByDocumentsResponse = $targetService->getCompletedTargetsByDocuments($getCompletedTargetsByDocumentsRequest);

  return $getCompletedTargetsByDocumentsResponse->return;
}

function downloadTargetResource($pdObject, $targetTicket) {
  $username = $pdObject->username;
  $password = $pdObject->password;
  $url = $pdObject->url;

  $sessionService = new SessionService2(GL_WSDL_PATH . 'SessionService2.wsdl', array(
              'location' => $url . "/services/SessionService2"
          ));
  $targetService = new TargetService2(GL_WSDL_PATH . 'TargetService2.wsdl', array(
              'location' => $url . "/services/TargetService2"
          ));
  $token = @Login($sessionService, $username, $password);
  $downloadTargetResourceRequest = new downloadTargetResource();

  $downloadTargetResourceRequest->targetId = $targetTicket;
  $downloadTargetResourceRequest->userId = $token;

  $downloadTargetResourceResponse = $targetService->downloadTargetResource($downloadTargetResourceRequest);
  $repositoryItem = $downloadTargetResourceResponse->return;
  $targetContent = $repositoryItem->data->_;

  return $targetContent;
}

function sendDownloadConfirmation($targetTicket, $pdObject) {
  $username = $pdObject->username;
  $password = $pdObject->password;
  $url = $pdObject->url;

  $sessionService = new SessionService2(GL_WSDL_PATH . 'SessionService2.wsdl', array(
              'location' => $url . "/services/SessionService2"
          ));
  $targetService = new TargetService2(GL_WSDL_PATH . 'TargetService2.wsdl', array(
              'location' => $url . "/services/TargetService2"
          ));
  $token = Login($sessionService, $username, $password);
  $sendDownloadConfirmationRequest = new sendDownloadConfirmation();

  $sendDownloadConfirmationRequest->targetId = $targetTicket;
  $sendDownloadConfirmationRequest->userId = $token;

  $sendDownloadConfirmationResponse = $targetService->sendDownloadConfirmation($sendDownloadConfirmationRequest);

  return $sendDownloadConfirmationResponse->return;
}
