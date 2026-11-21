USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPrivateChannels]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-07-11
description : ?? ?? ???? ?????.
result set : PrivateChannelID/bigunt, PrivateChannelName/nvarchar(10), MasterCharacterID/bigint, ChannelMemberCount/int, ChannelPassword/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103328 = ??? ????? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListPrivateChannels]	  
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
	/**_# [PrivateChannels] ???? SELECT???.*/
	;WITH PM AS (
		SELECT PrivateChannelID, COUNT(*) AS ChannelMemberCount
		FROM dbo.PrivateChannelMembers WITH (NOLOCK) 
		GROUP BY PrivateChannelID
	)
	SELECT PC.PrivateChannelID, PC.PrivateChannelName, PC.MasterCharacterID, ISNULL(PM.ChannelMemberCount, 0) AS ChannelMemberCount, PC.ChannelPassword
	FROM dbo.PrivateChannels PC WITH (NOLOCK, INDEX(PK_PrivateChannels)) LEFT OUTER JOIN PM ON PC.PrivateChannelID = PM.PrivateChannelID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103328;
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
