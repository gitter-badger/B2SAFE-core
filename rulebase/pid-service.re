################################################################################
#                                                                              #
# Hooks to the EPIC API, depend on wrapper scripts around the epicclient.py    #
# script                                                                       #
#                                                                              #
################################################################################

# Legend (the prefix i stands for iCAT and e for epic):
#
# ePID PID on the EPIC server
# iPID PID record in the iCAT
# eCHECKSUM checksum on the EPIC server
# iCHECKSUM checksum record on the iCAT

# List of the functions:
#
# EUDATCreatePID(*parent_pid, *path, *ror, *iCATCache, *newPID)
# EUDATSearchPID(*path, *existing_pid)
# EUDATSearchPIDchecksum(*path, *existing_pid)
# EUDATUpdatePIDWithNewChild(*parentPID, *childPID)
# EUDATGetRorPid(*pid, *ror)
# EUDATeiPIDeiChecksumMgmt(*path, *PID, *ePIDcheck, *iCATuse, *minTime)
# EUDATiPIDcreate(*path, *PID)
# EUDATiFieldVALUEretrieve(*path, *FNAME, *FVALUE)
# EUDATePIDcreate(*path, *PID)
# EUDATePIDsearch(*field, *value, *PID)
# EUDATeCHECKSUMupdate(*PID)
# EUDATeURLupdate(*PID, *newURL)
# EUDATePIDremove(*path)
# EUDATeiPIDeiChecksumMgmtColl(*sourceColl)
# EUDATiRORupdate(*source, *pid)
# EUDATeParentUpdate(*PID, *PFName, *PFValue)
# 

# Deprecated
# addPIDWithChecksum(*path, *newPID)

#
# Generate a new PID for a digital object.
# Fields stored in the PID record: URL, ROR and CHECKSUM
# adds a ROR field if (*ror != "None")
#
# Parameters:
#   *parent_pid [IN]    the PID of the digital object that was replicated to us (not necessarily the ROR)
#   *path       [IN]    the path of the replica to store with the PID record
#   *ror        [IN]    the ROR PID (absolute url) of the digital object that we want to store.
#   *iCATCache  [IN]    the boolean flag to enable creation of PID in the iCAT 
#   *newPID     [OUT]   the pid generated for this replica 
#
# Author: Willem Elbers, MPI-TLA
# Edited by Elena Erastova, RZG; Long Phan, JSC
#
# TODO: NEED TESTING
#
EUDATCreatePID(*parent_pid, *path, *ror, *iCATCache, *newPID) {
    logInfo("create pid for *path and save *ror as ror");
    getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);

    #check if PID already exists
    EUDATSearchPID(*path, *existing_pid);

    if((*existing_pid == "empty") || (*existing_pid == "None")) {
        # create PID
        EUDATePIDcreate(*path, *newPID);

        if (*iCATCache == bool("true")) {
            # Add PID into iCAT
            logInfo("Saving PID into field AVU -PID- of iCAT *path with PID = *newPID");
            EUDATiPIDcreate(*path, *newPID);
        }

    }
    else {
        *newPID = *existing_pid;
        logInfo("PID already exists (*newPID)");
    }

    # add RoR to PID record if there is one defined
#    if(*ror != "None") {
#        *listRor = split(*ror, "/");
#        *firstRor = elem(*listRor,0);
#       if(*firstRor != "http:") {
#            *ror = "*epicApi*ror";
#        }
#    }
#    if(*parent_pid != "None") {
#        *listPpid = split(*parent_pid, "/");
#        *firstPpid = elem(*listPpid,0);
#        if(*firstPpid != "http:") {
#            *parent_pid = "*epicApi*parent_pid";
#        }
#        if(*ror == "None") {
#            *ror = *parent_pid;
#            *parent_pid = "None";   
#        }
#        else if(*ror == *parent_pid) {
#            *parent_pid = "None";
#        }
#    }

#    if(*ror != "None") {
#        # add ROR to PID record
#        EUDATeParentUpdate(*newPID, "ROR", *ror);
#    }
#    if(*parent_pid != "None") {
#        # add PPID to PID record
#        EUDATeParentUpdate(*newPID, "PPID", *parent_pid);
#    }
     
     if (*ror != "None" && *ror != "" && *parent_pid != "None" && *parent_pid != "") {
                logInfo("Update Parent PID with fields ROR and PPID");

                EUDATeParentUpdate(*newPID, "ROR", *ror);
                EUDATeParentUpdate(*newPID, "PPID", *parent_pid);

     } else if ((*ror != "None" && *ror != "" && *parent_pid == "None") ||
                (*ror != "None" && *ror != "" && *parent_pid == "")) {

                logInfo("Update ParentPID with field ROR");
                EUDATeParentUpdate(*newPID, "ROR", *ror);

     } else if ((*parent_pid != "None" && *parent_pid != "" && *ror == "None") ||
                (*parent_pid != "None" && *parent_pid != "" && *ror == "")) {

                logInfo("Update ParentPID = *parent_pid with field PPID");
                EUDATeParentUpdate(*newPID, "PPID", *parent_pid);

     } else {
                logInfo("field ROR and PPID are empty or invalid, No ParentUpdate");
     }

}

#
# Searching for a PID using URL replacing "#", "%" and "&" with "*"
# (uses replaceHash in epicclient.py)
# Parameters:
#   *path       	[IN]    the path of the replica
#   *existing_pid	[OUT]   existing PID
#   *status             [REI]   false if no value is found, true elsewhere 
#
# Author: Elena Erastova, RZG
#
EUDATSearchPID(*path, *existing_pid) {
    logInfo("search pid for *path");
    getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
    msiExecCmd("epicclient.py","*credStoreType *credStorePath replaceHash *path", 
               "null", "null", "null", *out1);
    msiGetStdoutInExecCmdOut(*out1, *path1);
    logInfo("path = *path1, with serverID = *serverID");	
    *status = EUDATePIDsearch("URL", "*serverID"++"*path1", *existing_pid);
    logInfo("existing_pid = *existing_pid, with status = *status");	
    *status;
}

#
# Searching fo a PID using CHECKSUM
# Parameters:
#   *path       	[IN]    the path of the replica
#   *existing_pid	[OUT]   existing PID 
#
# Author: Elena Erastova, RZG
#
EUDATSearchPIDchecksum(*path, *existing_pid) {
    logInfo("search pid for *path");
    getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);

    #check if checksum already exists
    *checksum = "";
    msiSplitPath(*path,*parent,*child);
    EUDATiCHECKSUMretrieve(*path, *checksum);
    
    if(*checksum == "") {
        *existing_pid ="empty";
    }
    else {
        EUDATePIDsearch("CHECKSUM", *checksum, *existing_pid);
        msiExecCmd("epicclient.py", "*credStoreType *credStorePath read *existing_pid --key URL",
                   "null", "null", "null", *out);
        msiGetStdoutInExecCmdOut(*out, *URL);
        msiSplitPath(*URL,*parent1,*child1);
        if("*serverID*parent" != *parent1) {
            *existing_pid ="empty";
            logInfo("parent  = *serverID*parent ; parent1 = *parent1");
        }

        logInfo("search by CHECKSUM inside PID registry, got PID = *existing_pid");
    }
}

#
# Update a PID record with a new child.
#
# Parameters:
#       *parentPID  [IN]    PID of the record that will be updated
#       *childPID   [IN]    PID to store as one of the child locations
#
# Author: Willem Elbers, MPI-TLA
# Modified by: Claudio Cacciari, CINECA
#
EUDATUpdatePIDWithNewChild(*parentPID, *childPID) {
    logInfo("update parent pid (*parentPID) with new child (*childPID)");
    getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
    if(*epicDebug > 1) {
        logDebug("epicclient.py *credStoreType *credStorePath relation *parentPID *epicApi*childPID");
    }
    msiExecCmd("epicclient.py","*credStoreType *credStorePath relation *parentPID *epicApi*childPID",
               "null", "null", "null", *out);
    msiGetStdoutInExecCmdOut(*out, *response);
    if(*epicDebug > 1) {
    	logDebug("update handle location response = *response");
    }
}

#
# get the ROR entry for a PID
#
# Parameters:
#   *pid    [IN]     PID that you want to get the ROR for
#   *ror    [OUT]    ROR for *pid
#
EUDATGetRorPid(*pid, *ror) {
    logInfo("get RoR from (*pid)");
    getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
    msiExecCmd("epicclient.py", "*credStoreType *credStorePath read *pid --key EUDAT/ROR", 
               "null", "null", "null", *out);
    msiGetStdoutInExecCmdOut(*out, *ror);
    if(*ror=="None") {
        msiExecCmd("epicclient.py", "*credStoreType *credStorePath read *pid --key ROR", 
                   "null", "null", "null", *out);
        msiGetStdoutInExecCmdOut(*out, *ror);
        if(*ror=="None") {
            logInfo("getRorPID -> NO ROR for *pid ");
        }
    }
}

#
# This function create a PID for $objPath and store its value and the checksum in the iCAT if it does not exist.
# Otherwhise the function modify the PID.
#
# Environment variable used:
#   $objPath
#
# Arguments:
#   *path            [IN]    Path of the source file
#   *pid             [OUT]   PID of the source file
#   *ePIDcheck       [IN]    Specify whether you want to search for ePID (bool("true")) or not
#   *iCATuse         [IN]    Specify whether you want to use the iCAT (bool("true")) or not
#   *minTime         [IN]    Specify the minimum age of the digital object before looking for ePID
#
# Author: Giacomo Mariani, CINECA
#
EUDATeiPIDeiChecksumMgmt(*path, *PID, *ePIDcheck, *iCATuse, *minTime) {
    logInfo("EUDATeiPIDeiChecksumMgmt -> Look if the PID is in the iCAT");
    # Search for iPID and, if it exists, enter the if below
    if (EUDATiFieldVALUEretrieve(*path, "PID", *PID)) {
        logInfo("EUDATeiPIDeiChecksumMgmt -> Update PID with CHECKSUM for: 
                 *PID, $userNameClient, *path");
        EUDATeCHECKSUMupdate(*PID);                 
    }
    # iPID does not exist
    else {
        # If *ePIDcheck look for ePID
        *PID = "empty";
        if (*ePIDcheck) {
            logInfo("EUDATeiPIDeiChecksumMgmt -> No PID registered in iCAT. Looking on the EPIC server.");
            EUDATgetObjectTimeDiff(*path, "1", *liveTime);
            # If the file is older than "minTime" in seconds look for ePID
            if ( *liveTime >= *minTime ) {
                # Get ePID looking for one between: URL and CHECKSUM.
                EUDATSearchPID(*path, *PID);
                #EUDATSearchPIDchecksum(*path, *PID);
            }
        }
        # If ePID does not exist create both ePID and iPID
        if ( *PID == "empty" ) { 
            logInfo("EUDATeiPIDeiChecksumMgmt -> No PID in epic server yet");
            EUDATePIDcreate(*path, *newPID);
            if (*iCATuse == bool("true")) {
                # Add PID into iCAT
                EUDATiPIDcreate(*path, *newPID);
            }
        }
        else {
            logInfo("EUDATeiPIDeiChecksumMgmt -> Modifying the PID in epic server: *PID");  
            EUDATeCHECKSUMupdate(*PID);
            if (*iCATuse) {EUDATiPIDcreate(*path, *PID)};
        }
    }
}

#
# The function write iPID given ePID.
#
# Parameters:
#   *PID             [IN] ePID
#   *path            [IN] path of the object
#
# Author: Giacomo Mariani, CINECA
#
EUDATiPIDcreate(*path, *PID) {
    EUDATCreateAVU("PID", *PID, *path);
}

#
# The function retrieves the value of the required field.
#
# Arguments:
#   *path               [IN]    the iRODS path of the object involved in the query
#   *FNAME              [IN]    the name of the iCAT field the function is going to look for
#   *FVALUE             [OUT]   the corresponding value, if any
#   *status0            [REI]   false if no value is found, true elsewhere
#
# Author: Giacomo Mariani, CINECA
#
EUDATiFieldVALUEretrieve(*path, *FNAME, *FVALUE) {
    logInfo("EUDATiFieldVALUEretrieve -> looking for *FNAME of *path");
    *status0 = bool("false");
    msiSplitPath(*path, *coll, *name);
    *d = SELECT META_DATA_ATTR_VALUE WHERE DATA_NAME = '*name' AND COLL_NAME = '*coll' AND META_DATA_ATTR_NAME = '*FNAME'; 
    foreach(*c in *d) {
        msiGetValByKey(*c, "META_DATA_ATTR_VALUE", *FVALUE);
        logInfo("EUDATiFieldVALUEretrieve -> *FNAME equal to= *FVALUE");
        *status0 = bool("true");
    }
    *status0;
}

#
# The function create ePID.
#
# Environment variable used:
#
# Arguments:
#   *path            [IN]   The full iRODS path of the object
#   *PID             [OUT]    The created ePID.
#
# Author: Giacomo Mariani, CINECA
#
EUDATePIDcreate(*path, *PID) {
    getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug) ;
    logInfo("EUDATePIDcreate -> Add PID with CHECKSUM to: USER, OBJPATH: $userNameClient, *path");
    EUDATiCHECKSUMget(*path, *checksum);
    msiExecCmd("epicclient.py","*credStoreType *credStorePath create *serverID"++"*path --checksum *checksum",
               "null", "null", "null", *out);
    msiGetStdoutInExecCmdOut(*out, *PID);
    logInfo("EUDATePIDcreate -> Created handle is: *PID");
}

#
# The function retrieve ePID searching for a field between URL, CHECKSUM. 
#
# Environment variable used:
#   $objPath
#
# Arguments:
#   *field           [IN]    The eField to look in
#   *value           [IN]    The value to search for
#   *PID             [OUT]   ePID
#   *status0         [REI]   false if no value is found, true elsewhere
#
# Author: Giacomo Mariani, CINECA
#
EUDATePIDsearch(*field, *value, *PID) {
    logInfo("EUDATePIDsearch -> search the PID with *field equal to *value");
    getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
    *status0 = bool("true");
    msiExecCmd("epicclient.py","*credStoreType *credStorePath search *field *value", "null", "null", "null", *out);
    msiGetStdoutInExecCmdOut(*out, *PID);    
    logInfo("EUDATePIDsearch -> search handle response = ------ *PID ----------- ");
    logInfo("------------------------------------------------------------------- ");
    if ( str(*PID) == "empty" ) { 
        *status0=bool("false"); 
        logInfo("EUDATePIDsearch -> search handle response = no PID");
    }
    *status0;
}

#
# This function update the checksum field of the PID of $objPath  
#
# Arguments:
#   *PID                [IN] The PID associated to $objPath
#   *path               [IN] Object path
#
# Author: Giacomo Mariani, CINECA
#
EUDATeCHECKSUMupdate(*PID, *path) {
    getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug); 
    logInfo("EUDATeCHECKSUMupdate -> modify checksum related to PID *PID");
    EUDATiCHECKSUMget(*path, *checksum);
    msiExecCmd("epicclient.py","*credStoreType *credStorePath modify *PID CHECKSUM *checksum", "null", "null", "null", *out);
    msiGetStdoutInExecCmdOut(*out, *response);
    logInfo("EUDATeCHECKSUMupdate -> modify handle response = *response");
}

#
# This function update the URL field of the PID of $objPath
#
# Environment variable used:
#   $objPath        
#
# Arguments:
#   *PID                [IN] The PID associated to $objPath
#   *newURL             [IN] The new URL to be associated to the PID of $objPath
#
# Author: Giacomo Mariani, CINECA
#
EUDATeURLupdate(*PID, *newURL) {
    getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug) ;
    logInfo("EUDATeCHECKSUMupdate -> modify URL in PID *PID");
    msiExecCmd("epicclient.py","*credStoreType *credStorePath modify *PID URL \"*newURL\"",
               "null", "null", "null", *out);
    msiGetStdoutInExecCmdOut(*out, *response);
    logInfo("EUDATeCHECKSUMupdate -> modify handle response = *response");
#TODO the field 10320/loc needs to be updated too
}


#
# This function remove an ePID... even if its 10320/loc field is not empty!
# To be improved.       
#
# Arguments:
#   *path           [IN]    The path of the object to be removed
#
# Author: Giacomo Mariani, CINECA
#
EUDATePIDremove(*path) {
    getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug) 
    logInfo("EUDATePIDremove -> Removing PID associated to: $userNameClient, $objPath ($filePath)");

    if (EUDATSearchPID(*path, *pid)) {
        msiExecCmd("epicclient.py","*credStoreType *credStorePath read --key 10320/LOC *pid", "null", "null", "null", *out2);
        msiGetStdoutInExecCmdOut(*out2, *loc10320);
        logInfo("EUDATePIDremove -> get 10320/LOC from handle response = *loc10320");
        if (("*loc10320" like "Error*")||("*loc10320" == "")||("*loc10320" like "None*")) {
            logInfo("EUDATePIDremove -> 10320/LOC does not exist or is empty: PID will be deleted");
            msiExecCmd("epicclient.py","*credStoreType *credStorePath delete *pid",
                       "null", "null", "null", *out3);
            msiGetStdoutInExecCmdOut(*out3, *response3);
            logInfo("EUDATePIDremove -> delete handle response = *response3");
            # The PID record could be associated to a replica.
            # The field 10320/LOC of the parent PID record should be updated
        }
        else {
            # The PID record contains pointers to other DO copies.
            # What should we do?
            # Maybe all the copies should be deleted together with the master copy.
            logInfo("EUDATePIDremove -> The PID record *pid contains pointers to other DO copies");
        }
    }
    else {
        logInfo("EUDATePIDremove -> No PID associated to *path found");
    }
} 

#
# Walk through the collection. For each object in the collection 
# it creates a PID and stores its value and the object checksum in the iCAT if it does not exist.
# Otherwhise the function modify the PID.
#
# Arguments:
#   *sourceColl      [IN]    Source colection path
#
# Author: Elena Erastova, RZG
#
EUDATeiPIDeiChecksumMgmtColl(*sourceColl) {
    *Work=``{
        msiGetObjectPath(*File,*source,*status);
        logInfo("EUDATeiPIDeiChecksumMgmtColl: File *source");
	EUDATeiPIDeiChecksumMgmt(*source, *pid, bool("true"), bool("true"), 0);
        EUDATiRORupdate(*source, *pid);
        }``;
        msiCollectionSpider(*sourceColl,*File,*Work,*Status);
}

#
# Add the ROR field of the PID of the object to iCAT
#
# Arguments:
#   *source            [IN] Object path
#   *pid               [IN] Object pid
#
# Author: Elena Erastova, RZG
#
EUDATiRORupdate(*source, *pid) {
    if (!EUDATiFieldVALUEretrieve(*source, "EUDAT/ROR", *ror)) {
        getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
        logInfo("EUDATiRORupdate -> modify ROR in iCAT related to to the path *source");
        msiExecCmd("epicclient.py", "*credStoreType *credStorePath read *pid --key ROR", 
                   "null", "null", "null", *out);
        msiGetStdoutInExecCmdOut(*out, *ror);
        msiGetObjType(*source, *objType);
        if(*ror=="None") {
            msiExecCmd("epicclient.py", "*credStoreType *credStorePath read *pid --key EUDAT/ROR",
                       "null", "null", "null", *out);
            msiGetStdoutInExecCmdOut(*out, *ror);
            if(*ror=="None") {
                logInfo("EUDATiRORupdate -> NO ROR for *pid ");
            }
            else {
                *KVString="EUDAT/ROR=*ror";
                msiString2KeyValPair(*KVString,*KVPair);
                msiAssociateKeyValuePairsToObj(*KVPair,*source,*objType);
            }
        }
        else {
            *KVString="EUDAT/ROR=*ror";
            msiString2KeyValPair(*KVString,*KVPair);
            msiAssociateKeyValuePairsToObj(*KVPair,*source,*objType);
        }
        logInfo("EUDATiRORupdate -> saved ROR *ror for PID *pid ");
    }
}

#
# Update the EUDAT ROR or PPID field in the PID record
#
# Parameters:
#   *PID 	[IN]    the PID of the digital object whose PID record is updated
#   *PFName     [IN]    the name of the field of the PID record to update [ROR | PPID]
#   *PFValue    [IN]    the value of the field of the PID record to update.
#
# Author: Elena Erastova, RZG, edited by Claudio Cacciari, Cineca
#
EUDATeParentUpdate(*PID, *PFName, *PFValue) {
	
    getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
    # add Parent to PID record
    if(*epicDebug > 1) {
        logDebug("epicclient.py *credStoreType *credStorePath modify *PID *PFName *PFValue");
    }
    msiExecCmd("epicclient.py","*credStoreType *credStorePath modify *PID EUDAT/*PFName *PFValue",
               "null", "null", "null", *out);
    msiGetStdoutInExecCmdOut(*out, *response);
    if(*epicDebug > 1) {
        logDebug("modify handle response = *response")
    }
}

###############################################################################
# Remote operations on different scripts of B2Safe 
# (epicclient, authorization, logging)
#
###############################################################################

#
# Execute remote-action to  epicclient.script at destination Host
#   
# Parameters:
#   *srchost    [IN]    address of source host
#   *desthost   [IN]    address of destination host
#   *message    [IN]    message for field URL of PID
#   *out        [OUT]   pid
#
# Author: Long Phan, JSC
#
EUDATcreatePIDremote(*srchost, *desthost, *message) {
    remote(*desthost, "null") {
        logInfo(".... Accessing to remote-script epicclient at *desthost from *srchost, create pid with '*message' ");
        getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
        if (errorcode(msiExecCmd("epicclient.py", "*credStoreType *credStorePath create '*message'", "null","null","null",*out)) >= 0) {
            msiGetStdoutInExecCmdOut(*out, *result);
            remote(*srchost, "null") {
                logInfo(" Result: *result");
            }
        } else {
            remote(*srchost, "null") {
                logInfo(" Error, no PID is created ....");          
            }
        } 
    }
}

EUDATsearchPIDremote(*srchost, *desthost, *field, *value) {
    remote(*desthost, "null") {
        logInfo(".... Accessing to remote-script epicclient at *desthost from *srchost, search pid with *field and *value ");
        getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
        if (errorcode(msiExecCmd("epicclient.py", "*credStoreType *credStorePath search '*field' '*value'", "null", "null", "null",*out)) >= 0) {
            msiGetStdoutInExecCmdOut(*out, *result);
            remote(*srchost, "null") {
                logInfo(" Result: *result");
            }
        } else {
            remote(*srchost, "null") {
                logInfo(" Error, no PID is found ....");
            }
        }
    }
}

EUDATdeletePIDremote(*srchost, *desthost, *pid, *field) {
    remote(*desthost, "null") {
        logInfo(".... Accessing to remote-script epicclient at *desthost from *srchost, delete pid *field ");
        getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
        if (errorcode(msiExecCmd("epicclient.py", "*credStoreType *credStorePath delete *pid '*field'", "null", "null", "null", *out)) >= 0) {
            msiGetStdoutInExecCmdOut(*out, *result);
            remote(*srchost, "null") {
                logInfo(" Result: *result");
            }
        } else {
            remote(*srchost, "null") {
                logInfo(" Error, no PID is deleted ....");
            }
        }
    }
}

#
# Execute remote-action to Authorization-manager.script at destination Host
#   
# Parameters:
#   *srchost    [IN]    address of source host
#   *desthost   [IN]    address of destination host
#   *user       [IN]    irods-user want to be checked
#   *action     [IN]    the action want to be checked (ex. name of remote-script)
#   *target     [IN]    the target want to be checked (ex '* create *')
#
# Author: Long Phan, JSC
#
EUDATAuthZremote(*srchost, *desthost, *user, *action, *target) {
    remote(*desthost, "null") {
        logInfo(".... Accessing to remote-script Authorization-manager at *desthost from *srchost, check authorization of *user with *action to *target");
        getAuthZParameters(*authZMapPath);
        getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
        logInfo(" ....... *authZMapPath and *credStorePath ...... ");

       if (errorcode(msiExecCmd("authZ.manager.py", "*authZMapPath check *user '*action' '*target' '*credStorePath'", "null", "null", "null", *out)) >= 0) {
            msiGetStdoutInExecCmdOut(*out, *result);
            remote(*srchost, "null") {
                logInfo(" Result: *result");
            }
        } else {
            remote(*srchost, "null") {
                logInfo(" Error! Cannot execute remote script authZmanager.py or user is not identified");
            }
        }
    }
}

#
# Execute remote-action (queuesize) to logging.manager.script at destination Host
# Function can be extended to other actions (log, push, pop)
#   
# Parameters:
#   *srchost    [IN]    address of source host
#   *desthost   [IN]    address of destination host
#   *action     [IN]    the action want to be checked (ex. queuesize)
#
# Author: Long Phan, JSC
#
EUDATloggingremote(*srchost, *desthost, *action) {
    remote(*desthost, "null") {
        logInfo(".... Accessing to remote-script logmanager.py at *desthost from *srchost, execute logging with *action  ");
        getLogParameters(*logConfPath);
        if (errorcode(msiExecCmd("log.manager.py", "*logConfPath *action", "null", "null", "null", *out)) >= 0) {
            msiGetStdoutInExecCmdOut(*out, *result);
            remote(*srchost, "null") {
                logInfo(" Result: *result");
            }
        } else {
            remote(*srchost, "null") {
                logInfo(" Error! Cannot execute remote script logmanager.py or user is not identified");
            }
        }
    }
}

#
# DEMO FUNCTION: 
#       Create CPID and update PPID using remote-structure
# Parameters:
#   *srchost    [IN]    source host 
#   *desthost   [IN]    remote host
#   *ppid       [IN]    Parent-PID at source host which will be updated
#
EUDATdemoPIDremote(*srchost, *desthost, *ppid, *cpiddata) {
    logInfo("------- Begin DEMO create CPID at DESTINATION_HOST *desthost, update PPID *ppid at *srchost");
    remote(*desthost, "null") {        
        logInfo(".... Accessing to remote-script epicclient at DESTINATION_HOST *desthost, create pid with '*cpiddata' ");
        getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
        if (errorcode(msiExecCmd("epicclient.py", "*credStoreType *credStorePath create '*cpiddata'", "null","null","null",*out)) >= 0) {
            msiGetStdoutInExecCmdOut(*out, *cpid); 
            # improve with logging-script at *desthost           
            logInfo(" Result: *cpid");
            # msiSleep("10","20");
            remote(*srchost, "null") {
                getEpicApiParameters(*credStoreType, *credStorePath, *epicApi, *serverID, *epicDebug);
                if (errorcode(msiExecCmd("epicclient.py", "*credStoreType *credStorePath relation *ppid *epicApi*cpid", "null", "null", "null", *out)) >= 0) {
                    msiGetStdoutInExecCmdOut(*out, *response);
                    # improve with logging-script at *srchost
                    logInfo("Update PPID at SOURCE_HOST *srchost : *response");
                } else {
                    # improve with logging-script at *srchost
                    logInfo("Update PPID at SOURCE_HOST *srchost failed ...");
                }
            }
        } else {
            # improve with logging-script at *desthost
            logInfo(" Error, no PID is created at *desthost. So, Update PPID failed at *srchost ....");
        }
        
    }

}

################################################################################
#                                                                              #
# Deprecated functions                                                         #
#                                                                              #
################################################################################

#
# Parameters:
#   *path       [IN]    the path of the replica to store with the PID record
#   *newPID     [OUT]   the pid generated for this replica 
#
# Author: CINECA, edited and added by Elena Erastova, RZG
#
addPIDWithChecksum(*path, *newPID) {
	EUDATCreatePID("None", *path, "None", bool("false"), *newPID);
}
