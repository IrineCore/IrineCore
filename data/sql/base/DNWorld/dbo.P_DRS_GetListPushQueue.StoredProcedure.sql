USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListPushQueue]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-05-15
description : ?? ?? ?? ???? ?????.
result set : WishListID/int, CharacterID/bigint, AccountID/int, RegistedDate/datetime, PushTypeCode/tinyint, ItemID/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListPushQueue]	  
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbMaxID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY	
	
	IF OBJECT_ID(N'dbo.ReturnQueue', N'U') IS NOT NULL
		DROP TABLE dbo.ReturnQueue;	

	/**_# ?? PushQueue? ?? ?????? Bulk Insert???. */
	SELECT WishListID, CharacterID, RegistedDate, PushTypeCode, ItemID, QueueID, AccountID
	INTO dbo.ReturnQueue
	FROM dbo.PushQueue WITH (NOLOCK);

	IF @@ROWCOUNT = 0
	BEGIN
		SELECT WishListID, CharacterID, RegistedDate, PushTypeCode, ItemID
		FROM dbo.PushQueue
		WHERE 1 = 0;		
	END
	ELSE
	BEGIN
		/**_# ?????? ??? ???? Max ID ?? ????. */
		SELECT @inbMaxID = MAX(QueueID) 
		FROM dbo.ReturnQueue;	

		/**_# ?????? ??? ???? SELECT ???.*/
		SELECT WishListID, CharacterID, AccountID, RegistedDate, PushTypeCode, ItemID
		FROM dbo.ReturnQueue; 	

		/**_# PushQueue ????? ?????? ??? ???? DELETE ???.*/
		DELETE dbo.PushQueue
		WHERE QueueID <= @inbMaxID;	
	END

	/**_# ??? ?? ???? DROP ???.*/
	IF OBJECT_ID(N'dbo.ReturnQueue', N'U') IS NOT NULL
		DROP TABLE dbo.ReturnQueue;

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
