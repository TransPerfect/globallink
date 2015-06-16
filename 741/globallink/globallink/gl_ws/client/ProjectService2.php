<?php

if (!class_exists("anonymous0")) {

    class anonymous0 {
        
    }

}

if (!class_exists("base64Binary")) {

    class base64Binary {

        public $_; // base64Binary
        public $contentType; // anonymous0

    }

}

if (!class_exists("hexBinary")) {

    class hexBinary {

        public $_; // hexBinary
        public $contentType; // anonymous0

    }

}

if (!class_exists("Notification")) {

    class Notification {

        public $errorMessage; // string
        public $notificationDate; // Date
        public $notificationPriority; // NotificationPriority
        public $notificationText; // string

    }

}

if (!class_exists("NotificationPriority")) {

    class NotificationPriority {

        public $notificationPriorityName; // string

    }

}

if (!class_exists("Announcement")) {

    class Announcement {

        public $announcementText; // string
        public $date; // Date

    }

}

if (!class_exists("ContentMonitorPluginInfo")) {

    class ContentMonitorPluginInfo {

        public $pluginId; // string
        public $pluginName; // string

    }

}

if (!class_exists("Date")) {

    class Date {

        public $critical; // boolean
        public $date; // long

    }

}

if (!class_exists("Document")) {

    class Document {

        public $documentGroup; // DocumentGroup
        public $documentInfo; // DocumentInfo
        public $id; // string
        public $sourceLanguage; // Language
        public $sourceWordCount; // int
        public $ticket; // string

    }

}

if (!class_exists("DocumentGroup")) {

    class DocumentGroup {

        public $classifier; // string
        public $documents; // Document
        public $mimeType; // string
        public $submission; // Submission

    }

}

if (!class_exists("DocumentInfo")) {

    class DocumentInfo {

        public $clientIdentifier; // string
        public $dateRequested; // Date
        public $instructions; // string
        public $metadata; // Metadata
        public $name; // string
        public $projectTicket; // string
        public $sourceLocale; // string
        public $submissionTicket; // string
        public $targetInfos; // TargetInfo
        public $wordCount; // int

    }

}

if (!class_exists("DocumentPagedList")) {

    class DocumentPagedList {

        public $elements; // Document
        public $pagedListInfo; // PagedListInfo
        public $tasks; // Task
        public $totalCount; // long

    }

}

if (!class_exists("DocumentSearchRequest")) {

    class DocumentSearchRequest {

        public $projectTickets; // string
        public $sourceLocaleId; // string
        public $submissionTicket; // string

    }

}

if (!class_exists("DocumentTicket")) {

    class DocumentTicket {

        public $submissionTicket; // string
        public $ticketId; // string

    }

}

if (!class_exists("EntityTypeEnum")) {

    class EntityTypeEnum {

        public $name; // string
        public $value; // int

    }

}

if (!class_exists("FileFormatProfile")) {

    class FileFormatProfile {

        public $configurable; // boolean
        public $isDefault; // boolean
        public $mimeType; // string
        public $pluginId; // string
        public $pluginName; // string
        public $profileName; // string
        public $targetWorkflowDefinition; // WorkflowDefinition
        public $ticket; // string

    }

}

if (!class_exists("FileFormatProgressData")) {

    class FileFormatProgressData {

        public $dateCompleted; // Date
        public $fileCount; // long
        public $fileFormatName; // string
        public $fileProgressData; // FileProgressData
        public $jobTicket; // string
        public $workflowDueDate; // Date
        public $workflowStatus; // string

    }

}

if (!class_exists("FileProgressData")) {

    class FileProgressData {

        public $numberOfAvailableFiles; // int
        public $numberOfCanceledFiles; // int
        public $numberOfCompletedFiles; // int
        public $numberOfDeliveredFiles; // int
        public $numberOfFailedFiles; // int
        public $numberOfInProcessFiles; // int
        public $overallProgressPercent; // int

    }

}

if (!class_exists("FuzzyTmStatistics")) {

    class FuzzyTmStatistics {

        public $fuzzyName; // string
        public $wordCount; // int

    }

}

if (!class_exists("ItemFolderEnum")) {

    class ItemFolderEnum {

        public $value; // int

    }

}

if (!class_exists("ItemStatusEnum")) {

    class ItemStatusEnum {

        public $name; // string
        public $value; // int

    }

}

if (!class_exists("Metadata")) {

    class Metadata {

        public $key; // string
        public $value; // string

    }

}

if (!class_exists("Language")) {

    class Language {

        public $locale; // string
        public $value; // string

    }

}

if (!class_exists("LanguageDirection")) {

    class LanguageDirection {

        public $sourceLanguage; // Language
        public $targetLanguage; // Language

    }

}

if (!class_exists("LanguageDirectionModel")) {

    class LanguageDirectionModel {

        public $dateCompleted; // Date
        public $fileCount; // long
        public $fileFormatProgressData; // FileFormatProgressData
        public $fileProgress; // FileProgressData
        public $sourceLanguage; // Language
        public $targetLanguage; // Language
        public $workflowDueDate; // Date
        public $workflowStatus; // string

    }

}

if (!class_exists("PagedListInfo")) {

    class PagedListInfo {

        public $index; // int
        public $indexesSize; // int
        public $size; // int
        public $sortDirection; // string
        public $sortProperty; // string

    }

}

if (!class_exists("Phase")) {

    class Phase {

        public $dateEnded; // Date
        public $dueDate; // Date
        public $name; // string
        public $status; // ItemStatusEnum

    }

}

if (!class_exists("Priority")) {

    class Priority {

        public $name; // string
        public $value; // int

    }

}

if (!class_exists("Project")) {

    class Project {

        public $announcements; // Announcement
        public $contentMonitorPluginInfo; // ContentMonitorPluginInfo
        public $defaultLanguageDirections; // LanguageDirection
        public $defaultTargetWorkflowDefinition; // WorkflowDefinition
        public $defaultTargetWorkflowDefinitionTicket; // string
        public $fileFormatProfiles; // FileFormatProfile
        public $frequentLanguageDirections; // LanguageDirection
        public $metadata; // Metadata
        public $organizationName; // string
        public $projectInfo; // ProjectInfo
        public $projectLanguageDirections; // LanguageDirection
        public $ticket; // string
        public $workflowDefinitions; // WorkflowDefinition

    }

}

if (!class_exists("ProjectInfo")) {

    class ProjectInfo {

        public $clientIdentifier; // string
        public $defaultJobWorkflowDefinitionTicket; // string
        public $defaultSubmissionWorkflowDefinitionTicket; // string
        public $defaultTargetWorkflowDefinitionTicket; // string
        public $enabled; // boolean
        public $name; // string
        public $shortCode; // string

    }

}

if (!class_exists("ProjectLanguage")) {

    class ProjectLanguage {

        public $customLocaleCode; // string
        public $localeCode; // string

    }

}

if (!class_exists("RepositoryItem")) {

    class RepositoryItem {

        public $data; // base64Binary
        public $resourceInfo; // ResourceInfo

    }

}

if (!class_exists("ResourceInfo")) {

    class ResourceInfo {

        public $classifier; // string
        public $clientIdentifier; // string
        public $description; // string
        public $encoding; // string
        public $md5Checksum; // string
        public $mimeType; // string
        public $name; // string
        public $path; // string
        public $resourceInfoId; // long
        public $size; // long
        public $type; // ResourceType

    }

}

if (!class_exists("ResourceType")) {

    class ResourceType {

        public $value; // int

    }

}

if (!class_exists("Submission")) {

    class Submission {

        public $alerts; // Notification
        public $availableTasks; // int
        public $dateCompleted; // Date
        public $dateCreated; // Date
        public $dateEstimated; // Date
        public $documents; // Document
        public $dueDate; // Date
        public $id; // string
        public $owner; // string
        public $project; // Project
        public $status; // ItemStatusEnum
        public $submissionInfo; // SubmissionInfo
        public $submitter; // string
        public $submitterFullName; // string
        public $ticket; // string
        public $workflowDefinition; // WorkflowDefinition

    }

}

if (!class_exists("SubmissionInfo")) {

    class SubmissionInfo {

        public $clientIdentifier; // string
        public $dateRequested; // Date
        public $metadata; // Metadata
        public $name; // string
        public $projectTicket; // string
        public $submitter; // string
        public $workflowDefinitionTicket; // string

    }

}

if (!class_exists("SubmissionPagedList")) {

    class SubmissionPagedList {

        public $elements; // Submission
        public $pagedListInfo; // PagedListInfo
        public $tasks; // Task
        public $totalCount; // long

    }

}

if (!class_exists("SimpleSubmissionSearchModel")) {

    class SimpleSubmissionSearchModel {

        public $alerts; // Notification
        public $availableTasks; // long
        public $date; // Date
        public $dateCompleted; // Date
        public $dueDate; // Date
        public $fileCount; // long
        public $fileProgress; // FileProgressData
        public $id; // string
        public $instructions; // string
        public $officeName; // string
        public $owner; // string
        public $priority; // string
        public $projectName; // string
        public $projectTicket; // string
        public $sourceLanguage; // string
        public $status; // ItemStatusEnum
        public $submissionName; // string
        public $submitterFullName; // string
        public $ticket; // string
        public $wordCount; // long
        public $workflowDueDate; // Date
        public $workflowStatus; // string

    }

}

if (!class_exists("SubmissionSearchModelPagedList")) {

    class SubmissionSearchModelPagedList {

        public $elements; // SimpleSubmissionSearchModel
        public $pagedListInfo; // PagedListInfo
        public $tasks; // Task
        public $totalCount; // long

    }

}

if (!class_exists("SubmissionSearchRequest")) {

    class SubmissionSearchRequest {

        public $folder; // ItemFolderEnum
        public $projectTickets; // string
        public $submissionDate; // Date
        public $submissionDueDate; // Date
        public $submissionName; // string

    }

}

if (!class_exists("Target")) {

    class Target {

        public $availableTasks; // long
        public $dateCompleted; // Date
        public $dateCreated; // Date
        public $dateEstimated; // Date
        public $document; // Document
        public $downloadThresholdTimeStamp; // Date
        public $dueDate; // Date
        public $fileName; // string
        public $id; // string
        public $phases; // Phase
        public $refPhase; // Phase
        public $sourceLanguage; // Language
        public $sourceWordCount; // int
        public $status; // ItemStatusEnum
        public $targetInfo; // TargetInfo
        public $targetLanguage; // Language
        public $targetWordCount; // int
        public $ticket; // string
        public $tmStatistics; // TmStatistics
        public $workflowDefinition; // WorkflowDefinition

    }

}

if (!class_exists("TargetInfo")) {

    class TargetInfo {

        public $dateRequested; // Date
        public $encoding; // string
        public $instructions; // string
        public $metadata; // Metadata
        public $priority; // Priority
        public $requestedDueDate; // long
        public $targetLocale; // string
        public $workflowDefinitionTicket; // string

    }

}

if (!class_exists("TargetPagedList")) {

    class TargetPagedList {

        public $elements; // Target
        public $pagedListInfo; // PagedListInfo
        public $tasks; // Task
        public $totalCount; // long

    }

}

if (!class_exists("TargetSearchRequest")) {

    class TargetSearchRequest {

        public $dateCreated; // Date
        public $folder; // ItemFolderEnum
        public $projectTickets; // string
        public $sourceLocaleId; // string
        public $submissionTicket; // string
        public $targetLocaleId; // string

    }

}

if (!class_exists("Task")) {

    class Task {

        public $groupName; // string
        public $selectStyle; // int
        public $taskId; // long
        public $taskName; // string
        public $weight; // int

    }

}

if (!class_exists("TmStatistics")) {

    class TmStatistics {

        public $fuzzyWordCount1; // FuzzyTmStatistics
        public $fuzzyWordCount10; // FuzzyTmStatistics
        public $fuzzyWordCount2; // FuzzyTmStatistics
        public $fuzzyWordCount3; // FuzzyTmStatistics
        public $fuzzyWordCount4; // FuzzyTmStatistics
        public $fuzzyWordCount5; // FuzzyTmStatistics
        public $fuzzyWordCount6; // FuzzyTmStatistics
        public $fuzzyWordCount7; // FuzzyTmStatistics
        public $fuzzyWordCount8; // FuzzyTmStatistics
        public $fuzzyWordCount9; // FuzzyTmStatistics
        public $goldWordCount; // int
        public $noMatchWordCount; // int
        public $oneHundredMatchWordCount; // int
        public $repetitionWordCount; // int
        public $totalWordCount; // int

    }

}

if (!class_exists("WorkflowDefinition")) {

    class WorkflowDefinition {

        public $description; // string
        public $name; // string
        public $ticket; // string
        public $type; // EntityTypeEnum

    }

}

if (!class_exists("UserInfo")) {

    class UserInfo {

        public $accountNonExpired; // boolean
        public $accountNonLocked; // boolean
        public $autoClaimMultipleTasks; // boolean
        public $claimMultipleJobTasks; // boolean
        public $credentialsNonExpired; // boolean
        public $dateLastLogin; // dateTime
        public $emailAddress; // string
        public $emailNotification; // boolean
        public $enabled; // boolean
        public $firstName; // string
        public $lastName; // string
        public $password; // string
        public $timeZone; // string
        public $userName; // string
        public $userType; // string

    }

}

if (!class_exists("TiUserInfo")) {

    class TiUserInfo {

        public $languageDirections; // LanguageDirection
        public $organizationId; // long
        public $projectRoles; // string
        public $projectTicket; // string
        public $systemRoles; // string
        public $vendorId; // long

    }

}

if (!class_exists("findByTicket")) {

    class findByTicket {

        public $ticket; // string
        public $userId; // string

    }

}

if (!class_exists("findByTicketResponse")) {

    class findByTicketResponse {

        public $return; // Project

    }

}

if (!class_exists("findProjectByName")) {

    class findProjectByName {

        public $projectName; // string
        public $userId; // string

    }

}

if (!class_exists("findProjectByNameResponse")) {

    class findProjectByNameResponse {

        public $return; // Project

    }

}

if (!class_exists("findProjectByShortCode")) {

    class findProjectByShortCode {

        public $projectShortCode; // string
        public $userId; // string

    }

}

if (!class_exists("findProjectByShortCodeResponse")) {

    class findProjectByShortCodeResponse {

        public $return; // Project

    }

}

if (!class_exists("getUserProjects")) {

    class getUserProjects {

        public $isSubProjectIncluded; // boolean
        public $userId; // string

    }

}

if (!class_exists("getUserProjectsResponse")) {

    class getUserProjectsResponse {

        public $return; // Project

    }

}

/**
 * ProjectService2 class
 * 
 *  
 * 
 * @author    {author}
 * @copyright {copyright}
 * @package   {package}
 */
class ProjectService2 extends SoapClient {

    private static $classmap = array(
        'anonymous0' => 'anonymous0',
        'base64Binary' => 'base64Binary',
        'hexBinary' => 'hexBinary',
        'Notification' => 'Notification',
        'NotificationPriority' => 'NotificationPriority',
        'Announcement' => 'Announcement',
        'ContentMonitorPluginInfo' => 'ContentMonitorPluginInfo',
        'Date' => 'Date',
        'Document' => 'Document',
        'DocumentGroup' => 'DocumentGroup',
        'DocumentInfo' => 'DocumentInfo',
        'DocumentPagedList' => 'DocumentPagedList',
        'DocumentSearchRequest' => 'DocumentSearchRequest',
        'DocumentTicket' => 'DocumentTicket',
        'EntityTypeEnum' => 'EntityTypeEnum',
        'FileFormatProfile' => 'FileFormatProfile',
        'FileFormatProgressData' => 'FileFormatProgressData',
        'FileProgressData' => 'FileProgressData',
        'FuzzyTmStatistics' => 'FuzzyTmStatistics',
        'ItemFolderEnum' => 'ItemFolderEnum',
        'ItemStatusEnum' => 'ItemStatusEnum',
        'Metadata' => 'Metadata',
        'Language' => 'Language',
        'LanguageDirection' => 'LanguageDirection',
        'LanguageDirectionModel' => 'LanguageDirectionModel',
        'PagedListInfo' => 'PagedListInfo',
        'Phase' => 'Phase',
        'Priority' => 'Priority',
        'Project' => 'Project',
        'ProjectInfo' => 'ProjectInfo',
        'ProjectLanguage' => 'ProjectLanguage',
        'RepositoryItem' => 'RepositoryItem',
        'ResourceInfo' => 'ResourceInfo',
        'ResourceType' => 'ResourceType',
        'Submission' => 'Submission',
        'SubmissionInfo' => 'SubmissionInfo',
        'SubmissionPagedList' => 'SubmissionPagedList',
        'SimpleSubmissionSearchModel' => 'SimpleSubmissionSearchModel',
        'SubmissionSearchModelPagedList' => 'SubmissionSearchModelPagedList',
        'SubmissionSearchRequest' => 'SubmissionSearchRequest',
        'Target' => 'Target',
        'TargetInfo' => 'TargetInfo',
        'TargetPagedList' => 'TargetPagedList',
        'TargetSearchRequest' => 'TargetSearchRequest',
        'Task' => 'Task',
        'TmStatistics' => 'TmStatistics',
        'WorkflowDefinition' => 'WorkflowDefinition',
        'UserInfo' => 'UserInfo',
        'TiUserInfo' => 'TiUserInfo',
        'findByTicket' => 'findByTicket',
        'findByTicketResponse' => 'findByTicketResponse',
        'findProjectByName' => 'findProjectByName',
        'findProjectByNameResponse' => 'findProjectByNameResponse',
        'findProjectByShortCode' => 'findProjectByShortCode',
        'findProjectByShortCodeResponse' => 'findProjectByShortCodeResponse',
        'getUserProjects' => 'getUserProjects',
        'getUserProjectsResponse' => 'getUserProjectsResponse',
    );

    public function ProjectService2($wsdl = "http://localhost:8080/pd4/services/ProjectService2?wsdl", $options = array()) {
        foreach (self::$classmap as $key => $value) {
            if (!isset($options['classmap'][$key])) {
                $options['classmap'][$key] = $value;
            }
        }
        parent::__construct($wsdl, $options);
    }

    /**
     *  
     *
     * @param findByTicket $parameters
     * @return findByTicketResponse
     */
    public function findByTicket(findByTicket $parameters) {
        return $this->__soapCall('findByTicket', array($parameters), array(
            'uri' => 'http://impl.services2.service.ws.projectdirector.gs4tr.org',
            'soapaction' => ''
                )
        );
    }

    /**
     *  
     *
     * @param getUserProjects $parameters
     * @return getUserProjectsResponse
     */
    public function getUserProjects(getUserProjects $parameters) {
        return $this->__soapCall('getUserProjects', array($parameters), array(
            'uri' => 'http://impl.services2.service.ws.projectdirector.gs4tr.org',
            'soapaction' => ''
                )
        );
    }

    /**
     *  
     *
     * @param findProjectByShortCode $parameters
     * @return findProjectByShortCodeResponse
     */
    public function findProjectByShortCode(findProjectByShortCode $parameters) {
        return $this->__soapCall('findProjectByShortCode', array($parameters), array(
            'uri' => 'http://impl.services2.service.ws.projectdirector.gs4tr.org',
            'soapaction' => ''
                )
        );
    }

    /**
     *  
     *
     * @param findProjectByName $parameters
     * @return findProjectByNameResponse
     */
    public function findProjectByName(findProjectByName $parameters) {
        return $this->__soapCall('findProjectByName', array($parameters), array(
            'uri' => 'http://impl.services2.service.ws.projectdirector.gs4tr.org',
            'soapaction' => ''
                )
        );
    }

}
