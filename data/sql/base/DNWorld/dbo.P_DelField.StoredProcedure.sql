USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelField]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-13

description :
	???? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103281 = ???? ????.
history :
	2 = ??? ??? ?? ?? ??     by 20110419.wgkim
	4~6 = ??? ??? ?? ?? ??     by 20110914.wgkim
	7 = ?? ???? ???? ??? ???? ?? ?? ??     by 20110916.wgkim
**/

CREATE PROCEDURE [dbo].[P_DelField]
	  @intFarmID int --// ?? ID
	, @insFieldIndex smallint --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dtmLogDate datetime = GETDATE(), @intLogTableSuffix int, @xmlAttachments xml;

DECLARE @tblFieldDeleteLogs TABLE
( FarmID		int				NOT NULL
, FieldIndex	smallint		NOT NULL
, CharacterID	bigint			NOT NULL
, ItemID		int				NOT NULL
, PlantDate		datetime2(0)	NOT NULL
, ElapsedTime	int				NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dtmLogDate);

	SET @xmlAttachments = (
		SELECT AttachedItemID, AttachedDate
		FROM dbo.FieldAttachments WITH (NOLOCK)
		WHERE FarmID = @intFarmID AND FieldIndex = @insFieldIndex
		FOR XML AUTO, ROOT('root')
	);
	
	/**_# [FieldAttachments] ????? DELETE ???.*/
	DELETE dbo.FieldAttachments
	WHERE FarmID = @intFarmID AND FieldIndex = @insFieldIndex
	OPTION (KEEPFIXED PLAN);

	/**_# [Fields] ????? DELETE ???.*/
	DELETE dbo.Fields
	OUTPUT deleted.FarmID, deleted.FieldIndex, deleted.CharacterID, deleted.ItemID, deleted.PlantDate, deleted.ElapsedTime
	INTO @tblFieldDeleteLogs (FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime)
	WHERE FarmID = @intFarmID AND FieldIndex = @insFieldIndex
	OPTION (KEEPFIXED PLAN);

	/**_## ??? ???? ??? 103281? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103281;
		GOTO ErrorHandler;
	END
	
	/**_# [Fields] ??? ?? ??? ?????. */
	IF @intLogTableSuffix = 0
	BEGIN
		INSERT dbo.FieldDeleteLogs0 (LogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, HarvestFlag)
		SELECT @dtmLogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, 0 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);

		IF @xmlAttachments IS NOT NULL
		BEGIN
			INSERT dbo.FieldAttachmentDeleteLogs0 (LogDate, FarmID, FieldIndex, CharacterID, Attachments, HarvestFlag)
			SELECT @dtmLogDate, FarmID, FieldIndex, CharacterID, @xmlAttachments, 0 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);
		END
	END
	ELSE IF @intLogTableSuffix = 1
	BEGIN
		INSERT dbo.FieldDeleteLogs1 (LogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, HarvestFlag)
		SELECT @dtmLogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, 0 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);

		IF @xmlAttachments IS NOT NULL
		BEGIN
			INSERT dbo.FieldAttachmentDeleteLogs1 (LogDate, FarmID, FieldIndex, CharacterID, Attachments, HarvestFlag)
			SELECT @dtmLogDate, FarmID, FieldIndex, CharacterID, @xmlAttachments, 0 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);
		END
	END
	ELSE
	BEGIN
		INSERT dbo.FieldDeleteLogs2 (LogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, HarvestFlag)
		SELECT @dtmLogDate, FarmID, FieldIndex, CharacterID, ItemID, PlantDate, ElapsedTime, 0 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);

		IF @xmlAttachments IS NOT NULL
		BEGIN
			INSERT dbo.FieldAttachmentDeleteLogs2 (LogDate, FarmID, FieldIndex, CharacterID, Attachments, HarvestFlag)
			SELECT @dtmLogDate, FarmID, FieldIndex, CharacterID, @xmlAttachments, 0 FROM @tblFieldDeleteLogs OPTION (KEEPFIXED PLAN);
		END
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
