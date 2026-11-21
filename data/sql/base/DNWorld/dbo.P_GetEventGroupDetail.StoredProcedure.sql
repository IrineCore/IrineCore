USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetEventGroupDetail]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2011-07-11
description : ??? ?? ????? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetEventGroupDetail]
	  @intEventGroupID int --// ??? ?? ID	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF NOT EXISTS ( SELECT * FROM dbo.EventGroups WHERE EventGroupID = @intEventGroupID )
	BEGIN
		SET @intReturnValue = 103177;
		GOTO ErrorHandler;
	END
	
	SELECT EL.EventGroupID, EL.EventIndex AS EventID, Left(WID.WorldIDs,Len(WID.WorldIDs)-1) AS WorldID, EL.MapIDs, EL.EventStartDate, EL.EventEndDate, EL.EventType1, EL.EventType2, EL.EventType3, EL.EventAtt1, EL.EventAtt2, EL.EventAtt3, EL.EventAtt4, EL.EventAtt5
	FROM
	(
		SELECT EventGroupID, EventIndex, MapIDs, EventStartDate, EventEndDate, EventType1, EventType2, EventType3, EventAtt1, EventAtt2, EventAtt3, EventAtt4, EventAtt5 FROM dbo.EventStatus
		WHERE EventGroupID = @intEventGroupID 
			and WorldID = (SELECT MIN(WorldID) FROM dbo.EventStatus WHERE EventGroupID = @intEventGroupID)
	) EL 
	LEFT OUTER JOIN
	(
		SELECT distinct ES2.EventGroupID,
		(
			SELECT CAST(ES.WorldID AS varchar) + ',' AS [text()]
			FROM (SELECT EventGroupID, WorldID
					FROM EventStatus				
					GROUP BY EventGroupID, WorldID) AS ES
			WHERE ES.EventGroupID = ES2.EventGroupID
			ORDER BY ES.EventGroupID
			FOR XML PATH ('')
		) WorldIDs
		FROM EventStatus ES2	
	)
	WID
	ON EL.EventGroupID = WID.EventGroupID

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
