USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMailAttachment]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-03
description : ??? ??? ??? ??? ?? ?? ??? ?????.
result set : 
	version 6 : MailID/int, AttachCoin/bigint, MailAttachItemCount/int
	version 8 : MailID/int, AttachCoin/bigint, MailAttachItemCount/int, MailTypeCode/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5 = TF_Split? IF_Split?? ?? at 2010-12-21
6 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
8 = input param. @intVersion ?? by ??? at 2012-09-12
**/

CREATE PROCEDURE [dbo].[P_GetListMailAttachment]
	  @inbReceiverCharacterID bigint --// ?? ??? ID
	, @vchMailIDs varchar(8000) --// ,? ?????? ?? ID ???
	, @intVersion int = 6 --// ??? SP? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblMailIDs table (MailID int NOT NULL PRIMARY KEY);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblMailIDs (MailID)
	SELECT CAST(string AS int)
	FROM dbo.TF_Split(@vchMailIDs, ',')
	WHERE string > ''
	OPTION (KEEPFIXED PLAN);

	IF @intVersion = 6
		WITH C1 AS (
			SELECT M.MailID, CASE M.AttachCoinReceiveFlag WHEN 1 THEN 0 ELSE M.AttachCoin END AS AttachCoin, ISNULL(CAST(V.MailAttachItemCount AS int), 0) AS MailAttachItemCount
			FROM dbo.Mails M WITH (NOLOCK)
				INNER JOIN @tblMailIDs T ON M.MailID = T.MailID
				LEFT OUTER JOIN dbo.VIX_MailAttachItemCounts V WITH (NOEXPAND, NOLOCK) ON M.MailID = V.MailID AND V.ReceiveFlag = CAST(0 AS bit)
			WHERE M.ReceiverCharacterID = @inbReceiverCharacterID
		)
		SELECT MailID, AttachCoin, MailAttachItemCount
		FROM C1
		WHERE AttachCoin > 0 OR MailAttachItemCount > 0
		ORDER BY MailID DESC;

	ELSE IF @intVersion = 8
		WITH C1 AS (
			SELECT M.MailID, CASE M.AttachCoinReceiveFlag WHEN 1 THEN 0 ELSE M.AttachCoin END AS AttachCoin, ISNULL(CAST(V.MailAttachItemCount AS int), 0) AS MailAttachItemCount, M.MailTypeCode
			FROM dbo.Mails M WITH (NOLOCK)
				INNER JOIN @tblMailIDs T ON M.MailID = T.MailID
				LEFT OUTER JOIN dbo.VIX_MailAttachItemCounts V WITH (NOEXPAND, NOLOCK) ON M.MailID = V.MailID AND V.ReceiveFlag = CAST(0 AS bit)
			WHERE M.ReceiverCharacterID = @inbReceiverCharacterID
		)
		SELECT MailID, AttachCoin, MailAttachItemCount, MailTypeCode
		FROM C1
		WHERE AttachCoin > 0 OR MailAttachItemCount > 0
		ORDER BY MailID DESC;
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
