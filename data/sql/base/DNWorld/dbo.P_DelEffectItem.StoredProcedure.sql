USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelEffectItem]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-24
description : ?? ???? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DelEffectItem]
	  @intEffectItemID int --// ??? ?? ?? ??? ID
	, @nvcModifier nvarchar(50) --// ??? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE
	  @nvcTableName nvarchar(128)		-- The name of table.
	, @inyOperationCode tinyint			-- 1=Create, 2=Retrieve, 3=Update, 4=Delete.
	, @nvcColumnNames nvarchar(max)		-- The names of the columns affected by a operation. It was delimitated by [|].
	, @nvcBeforeImages nvarchar(max)	-- The contents of the columns before operation. It was delimitated by [|].
	, @dtmOccurredDate datetime;		-- The date the modification occurred.

DECLARE @tblBeforeImages table (beforeImage nvarchar(max));

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# AUDIT? ?? ???? ?? ???? ?????.*/
	SET @nvcTableName = N'dbo.EffectItems';
	SET @inyOperationCode = 4;
	SET @nvcColumnNames = N'EffectItemID|CharacterID|ItemID|EffectItemGetCode|EffectItemGetKey|Price|EternityFlag|EffectItemExpireDate|Property1|Property2|Property3|Property4|Property5|RegisterDate|ExpireCompleteFlag|ItemSerial';
	SET @dtmOccurredDate = GETDATE();

	BEGIN TRANSACTION;
		/**_# ?? ????, ?? ? ???? ??? ??? ?? ?????.*/
		DELETE dbo.EffectItems
		OUTPUT ISNULL(CAST(DELETED.EffectItemID AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.CharacterID AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.ItemID AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.EffectItemGetCode AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.EffectItemGetKey AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.Price AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.EternityFlag AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.EffectItemExpireDate AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.Property1 AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.Property2 AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.Property3 AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.Property4 AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.Property5 AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.RegisterDate AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.ExpireCompleteFlag AS nvarchar(max)), N'[NULL]') + N'|' + ISNULL(CAST(DELETED.ItemSerial AS nvarchar(max)), N'[NULL]')
			INTO @tblBeforeImages
		WHERE EffectItemID = @intEffectItemID;

		/**_# ?? ? ???? [TableAuditLogs] ???? INSERT???.*/
		SELECT @nvcBeforeImages = beforeImage FROM @tblBeforeImages;

		EXEC @intReturnValue = dbo.P_AddAuditLog
			  @nvcTableName = @nvcTableName
			, @inyOperationCode = @inyOperationCode
			, @nvcColumnNames = @nvcColumnNames
			, @nvcBeforeImages = @nvcBeforeImages
			, @nvcModifier = @nvcModifier
			, @dtmOccurredDate = @dtmOccurredDate;

		IF @intReturnValue <> 0 GOTO ErrorHandler;
	COMMIT TRANSACTION;
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
