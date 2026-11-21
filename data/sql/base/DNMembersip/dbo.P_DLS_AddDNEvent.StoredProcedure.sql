USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_AddDNEvent]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DLS_AddDNEvent]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DLS_AddDNEvent] AS' 
END
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-11-08

description :
	??? ??
result set :
	@intDNEventID / int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101214 = ??? ??? ???????.
	101215 = ??? ?? ??? ???????.
history :
	3 = ????
**/

ALTER PROCEDURE [dbo].[P_DLS_AddDNEvent]
	  @intDNEventDetailID int --// ??? ?? ID
	, @inyWorldID tinyint --// ?? ID (0=??)
	, @bitEnableFlag bit = 1 --// 0=???, 1=??
	, @sdtRegistrationDate smalldatetime --// ?? ??
	, @xmlEventSchedule xml --// ??? ?? '<root><EventSchedule EventStartDate="" EventEndDate="" /></root>'
	, @intDNEventID int = NULL OUTPUT --// ??? ID
	, @bitDisplayFlag bit = 0 --// 0=??? ?? ???, 1=??? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intDoc int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ??? ?? */
	INSERT dbo.DNEvents (DNEventDetailID, WorldID, EnableFlag, DisplayFlag)
	VALUES (@intDNEventDetailID, @inyWorldID, @bitEnableFlag, @bitDisplayFlag);

	SET @intDNEventID = SCOPE_IDENTITY();

	IF @intDNEventID IS NOT NULL
	BEGIN
		EXEC sys.sp_xml_preparedocument @intDoc OUTPUT, @xmlEventSchedule;

		WITH T AS (
			SELECT EventStartDate, EventEndDate
			FROM OPENXML (@intDoc, 'root/EventSchedule')
			WITH (
				EventStartDate smalldatetime  '@EventStartDate',
				EventEndDate smalldatetime  '@EventEndDate'
			)
		)
		INSERT dbo.DNEventSchedules (EventEndDate, EventStartDate, DNEventID)
		SELECT EventEndDate, EventStartDate, @intDNEventID FROM T;

		IF @@ROWCOUNT = 0
		BEGIN
			EXEC sys.sp_xml_removedocument @intDoc;

			SET @intReturnValue = 101215;
			GOTO ErrorHandler;
		END

		EXEC sys.sp_xml_removedocument @intDoc;
	END
	ELSE
	BEGIN
		SET @intReturnValue = 101214;
		GOTO ErrorHandler;
	END

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
