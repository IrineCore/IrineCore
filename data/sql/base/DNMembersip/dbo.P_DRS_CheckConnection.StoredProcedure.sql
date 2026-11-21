USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_CheckConnection]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_CheckConnection]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_CheckConnection] AS' 
END
GO
/**
version : 8
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-04-26
description : ???? ????? ?????.
result set : CharacterID/int, WorldID/int, CurServerID/int, CertifyingStep/int, IsRestraint/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DRS_CheckConnection]	  
	  @vchCharacterIDs varchar(max) --// ??? ID ???. ???? ,???.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @bitMaintenanceFlag bit = NULL;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @bitMaintenanceFlag = ISNULL(MaintenanceFlag, CAST(0 AS bit)) FROM dbo.MaintenanceFlag WITH (NOLOCK);

	IF @bitMaintenanceFlag = CAST(0 AS bit) OR @bitMaintenanceFlag IS NULL
		WITH AID AS (
		SELECT CH.AccountID, CH.CharacterID, CASE WHEN R.AccountID IS NOT NULL THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END AS IsRestraint
		FROM dbo.TF_Split(@vchCharacterIDs, ',') T
			 INNER JOIN dbo.Characters CH WITH (NOLOCK) ON CAST(T.string AS bigint) = CH.CharacterID
			 LEFT OUTER JOIN dbo.Restraints R WITH (NOLOCK) ON CH.AccountID = R.AccountID AND (R.StartDate < GETDATE() AND R.EndDate > GETDATE()) AND CancelFlag = 0
		)
		SELECT AID.CharacterID, ISNULL(AU.WorldID, 0) AS WorldID, ISNULL(AU.CurServerID, 0) AS CurServerID, ISNULL(AU.CertifyingStep, 0) AS CertifyingStep, IsRestraint
		FROM AID LEFT OUTER JOIN dbo.DNAuth AU WITH (NOLOCK) ON AID.AccountID = AU.AccountDBID AND AID.CharacterID = AU.CharacterDBID;		
	ELSE
		WITH AID AS (
		SELECT CH.AccountID, CH.CharacterID, CASE WHEN R.AccountID IS NOT NULL THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END AS IsRestraint
		FROM dbo.TF_Split(@vchCharacterIDs, ',') T
			 INNER JOIN dbo.Characters CH WITH (NOLOCK) ON CAST(T.string AS bigint) = CH.CharacterID
			 LEFT OUTER JOIN dbo.Restraints R WITH (NOLOCK) ON CH.AccountID = R.AccountID AND (R.StartDate < GETDATE() AND R.EndDate > GETDATE()) AND CancelFlag = 0
		)
		SELECT AID.CharacterID, 0 AS WorldID, 0 AS CurServerID, 0 AS CertifyingStep, IsRestraint
		FROM AID LEFT OUTER JOIN dbo.DNAuth AU WITH (NOLOCK) ON AID.AccountID = AU.AccountDBID AND AID.CharacterID = AU.CharacterDBID;		
	
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
