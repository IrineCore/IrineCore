USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetPageReceiveMail]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-12-14
description : (????) ???? ?????.
result set : 
		RowNumber/int, MailID/int, SenderCharacterName/nvarchar, Subject/nvarchar, RemainDay/int, ReadFlag/bit, DeliveryTypeCode/tinyint, MailTypeCode/tinyint, AttachmentFlag/bit, NewFlag/bit, SendDate/datetime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetPageReceiveMail]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @inbReceiverCharacterID bigint --// ?? ??? ID
	, @intTotalMailCount int OUTPUT --// ? ?? ?? ?
	, @intNotReadMailCount int OUTPUT --// ?? ?? ?? ?
	, @int7DaysLeftMailCount int OUTPUT --// ?? ??? 7? ?? ?? ?
	, @intBasicMailCount int OUTPUT --// ?? ?? ?? ?? ?
	, @inyMailTabCode tinyint = NULL --// ?? ? ??	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	EXEC @intReturnValue = dbo.P_GetCountReceiveMail
		  @inbReceiverCharacterID = @inbReceiverCharacterID
		, @intTotalMailCount = @intTotalMailCount OUTPUT
		, @intNotReadMailCount = @intNotReadMailCount OUTPUT
		, @int7DaysLeftMailCount = @int7DaysLeftMailCount OUTPUT;

	IF @inyMailTabCode IS NULL
		SELECT @intBasicMailCount = COUNT(*)
		FROM dbo.Mails WITH (NOLOCK, INDEX(IX_NNI_Mails_SenderCharacterID), FORCESEEK)
		WHERE SenderCharacterID = @inbReceiverCharacterID AND DeliveryTypeCode = 1 AND SendDate >= CAST(CAST(GETDATE() AS date) AS datetime2(0))
		OPTION (KEEPFIXED PLAN);
	ELSE
		SELECT @intBasicMailCount = COUNT(*)
		FROM dbo.Mails WITH (NOLOCK, INDEX(IX_NNI_Mails_SenderCharacterID), FORCESEEK)
		WHERE SenderCharacterID = @inbReceiverCharacterID AND DeliveryTypeCode = 1 AND SendDate >= CAST(CAST(GETDATE() AS date) AS datetime2(0)) AND MailTabCode = @inyMailTabCode
		OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
		SET @intBasicMailCount = 0;

	IF @inyMailTabCode IS NULL
		WITH PagingCTE AS (
			SELECT ROW_NUMBER() OVER(ORDER BY MailID DESC) AS RowNumber, MailID
			FROM dbo.Mails WITH (NOLOCK, INDEX(IX_NNI_Mails_ReceiverCharacterID), FORCESEEK)
			WHERE ReceiverCharacterID = @inbReceiverCharacterID AND DeleteFlag = CAST(0 AS bit) AND SendDate >= CAST(CAST(DATEADD(DAY, -29, @dt2Now) AS date) AS datetime2(0)) 
		)
		SELECT C1.RowNumber, M.MailID, (CASE WHEN M.MailTypeCode = 19 THEN M.SenderName ELSE COALESCE(CH.CharacterName, M.SenderName) END) AS SenderCharacterName, M.[Subject], 30 - DATEDIFF(DAY, M.SendDate, @dt2Now) AS RemainDay, M.ReadFlag, M.DeliveryTypeCode, M.MailTypeCode
			, CASE
				WHEN (M.AttachCoin > 0 AND M.AttachCoinReceiveFlag = CAST(0 AS bit)) OR EXISTS (SELECT * FROM dbo.MailAttachItems WITH (NOLOCK, INDEX(PK_MailAttachItems), FORCESEEK) WHERE MailID = C1.MailID AND ReceiveFlag = CAST(0 AS bit)) THEN CAST(1 AS bit)
				ELSE CAST(0 AS bit)
			END AS AttachmentFlag, ISNULL(M.NewFlag, CAST(0 AS bit)) AS NewFlag, M.SendDate
		FROM PagingCTE C1
			INNER JOIN dbo.Mails M WITH (NOLOCK, INDEX(PK_Mails), FORCESEEK) ON C1.MailID = M.MailID
			LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON M.SenderCharacterID = CH.CharacterID
		WHERE RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
		ORDER BY RowNumber
		OPTION (KEEPFIXED PLAN, MAXDOP 1);
	ELSE
		WITH PagingCTE AS (
			SELECT ROW_NUMBER() OVER(ORDER BY MailID DESC) AS RowNumber, MailID
			FROM dbo.Mails WITH (NOLOCK, INDEX(IX_NNI_Mails_ReceiverCharacterID), FORCESEEK)
			WHERE ReceiverCharacterID = @inbReceiverCharacterID AND DeleteFlag = CAST(0 AS bit) AND SendDate >= CAST(CAST(DATEADD(DAY, -29, @dt2Now) AS date) AS datetime2(0)) AND MailTabCode = @inyMailTabCode
		)
		SELECT C1.RowNumber, M.MailID, (CASE WHEN M.MailTypeCode = 19 THEN M.SenderName ELSE COALESCE(CH.CharacterName, M.SenderName) END) AS SenderCharacterName, M.[Subject], 30 - DATEDIFF(DAY, M.SendDate, @dt2Now) AS RemainDay, M.ReadFlag, M.DeliveryTypeCode, M.MailTypeCode
			, CASE
				WHEN (M.AttachCoin > 0 AND M.AttachCoinReceiveFlag = CAST(0 AS bit)) OR EXISTS (SELECT * FROM dbo.MailAttachItems WITH (NOLOCK, INDEX(PK_MailAttachItems), FORCESEEK) WHERE MailID = C1.MailID AND ReceiveFlag = CAST(0 AS bit)) THEN CAST(1 AS bit)
				ELSE CAST(0 AS bit)
			END AS AttachmentFlag, ISNULL(M.NewFlag, CAST(0 AS bit)) AS NewFlag, M.SendDate
		FROM PagingCTE C1
			INNER JOIN dbo.Mails M WITH (NOLOCK, INDEX(PK_Mails), FORCESEEK) ON C1.MailID = M.MailID
			LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON M.SenderCharacterID = CH.CharacterID
		WHERE RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
		ORDER BY RowNumber
		OPTION (KEEPFIXED PLAN, MAXDOP 1);
	
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
