USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMailForGSM]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-16
description : ?/??? ?? ??? ?????.
result set : MailID/int, SenderCharacterID/bigint, SenderCharacterName/nvarchar, ReceiverCharacterID/bigint, ReceiverCharacterName/nvarchar, MailTypeCode/tinyint, DeliveryTypeCode/tinyint, Subject/nvarchar, Content/nvarchar, SendDate/datetime2, ReadFlag/bit, DeleteFlag/bit, AttachCoin/bigint, AttachCoinReceiveFlag/bit, SenderName/nvarchar, SystemMailFKey/bigint, AttachItemFlag/bit, DeleteDate/datetime2
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103199 = ??? ???? ??? ????.
history :
2 = ?????? ??? ?????. at 2010-10-19
3 = ?? ??? DeleteDate ?? at 2010-11-18
4~5 = (44097) result ?? AttachItemFlag ? ??? ???? ????? ??? ?? by ??? at 2011-10-21
**/

CREATE PROCEDURE [dbo].[P_GetListMailForGSM]
	  @inbSenderCharacterID bigint = NULL --// ?? ??? ID
	, @inbReceiverCharacterID bigint = NULL --// ?? ??? ID
	, @inyMailTypeCode tinyint = NULL --// 1=?? ??, 2=?? ??, 3=??? ??
	, @inyDeliveryTypeCode tinyint = NULL --// 1=??, 2=????
	, @bitReadFlag bit = NULL --// 0=????, 1=??
	, @bitDeleteFlag bit = NULL --// 0=???, 1=??
	, @dt2StartDate datetime2(0) = NULL --// ?? ?? ??. ?? ?? ??
	, @dt2EndDate datetime2(0) = NULL --// ?? ? ??. ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @intColumnCount = 8;

	SET @inbNullBitmap = 
		CASE WHEN @inbSenderCharacterID IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @inbReceiverCharacterID IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @inyMailTypeCode IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @inyDeliveryTypeCode IS NOT NULL THEN POWER(2, 3) ELSE 0 END + 
		CASE WHEN @bitReadFlag IS NOT NULL THEN POWER(2, 4) ELSE 0 END + 
		CASE WHEN @bitDeleteFlag IS NOT NULL THEN POWER(2, 5) ELSE 0 END +
		CASE WHEN @dt2StartDate IS NOT NULL THEN POWER(2, 6) ELSE 0 END + 
		CASE WHEN @dt2EndDate IS NOT NULL THEN POWER(2, 7) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @nvcStmt = N'
SELECT DISTINCT M.MailID, M.SenderCharacterID, CH1.CharacterName AS SenderCharacterName, M.ReceiverCharacterID, CH2.CharacterName AS ReceiverCharacterName
	, M.MailTypeCode, M.DeliveryTypeCode, M.Subject, M.Content, M.SendDate, M.ReadFlag, M.DeleteFlag, M.AttachCoin, M.AttachCoinReceiveFlag
	, M.SenderName, M.SystemMailFKey, CASE WHEN MAI.MailID IS NULL THEN CAST(0 AS bit) ELSE CAST(1 AS bit) END AS AttachItemFlag
	, M.DeleteDate
FROM dbo.Mails M WITH (NOLOCK)
	LEFT OUTER JOIN dbo.Characters CH1 WITH (NOLOCK) ON M.SenderCharacterID = CH1.CharacterID
	LEFT OUTER JOIN dbo.Characters CH2 WITH (NOLOCK) ON M.ReceiverCharacterID = CH2.CharacterID
	LEFT OUTER JOIN dbo.MailAttachItems MAI WITH (NOLOCK) ON M.MailID = MAI.MailID
'		+ dbo.FN_DynamicWhereClause2(N'M.SenderCharacterID', N'@inbSenderCharacterID', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'M.ReceiverCharacterID', N'@inbReceiverCharacterID', @inbNullBitmap, 2, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'M.MailTypeCode', N'@inyMailTypeCode', @inbNullBitmap, 3, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'M.DeliveryTypeCode', N'@inyDeliveryTypeCode', @inbNullBitmap, 4, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'M.ReadFlag', N'@bitReadFlag', @inbNullBitmap, 5, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'M.DeleteFlag', N'@bitDeleteFlag', @inbNullBitmap, 6, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'M.SendDate', N'@dt2StartDate', @inbNullBitmap, 7, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'M.SendDate', N'@dt2EndDate', @inbNullBitmap, 8, @intColumnCount, '<=') + N'
ORDER BY M.MailID DESC';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'@inbSenderCharacterID bigint
		, @inbReceiverCharacterID bigint
		, @inyMailTypeCode tinyint
		, @inyDeliveryTypeCode tinyint
		, @bitReadFlag bit, @bitDeleteFlag bit
		, @dt2StartDate datetime2(0)
		, @dt2EndDate datetime2(0)'
		, @inbSenderCharacterID = @inbSenderCharacterID
		, @inbReceiverCharacterID = @inbReceiverCharacterID
		, @inyMailTypeCode = @inyMailTypeCode
		, @inyDeliveryTypeCode = @inyDeliveryTypeCode
		, @bitReadFlag = @bitReadFlag
		, @bitDeleteFlag = @bitDeleteFlag
		, @dt2StartDate = @dt2StartDate
		, @dt2EndDate = @dt2EndDate;

	/**_## ??? ???? ?? ???? ???, 103199? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103199;
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
