USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFanRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-02
description : ??? ? ??? ?????. (TOP 12)
result set : CharacterName/nvarchar, Intimacy/int, UpdateDate/datetime2
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListFanRanking]	  
	  @inbDNGirlCharacterID bigint --// ??? ???ID	
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
	/**_# [DNGirlIntimacy] ???? SELECT???.*/
	SELECT TOP 12 CH.CharacterName, GI.Intimacy, GI.UpdateDate
	FROM dbo.DNGirlIntimacy GI WITH (NOLOCK) 
	     INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK) ON GI.CharacterID = CH.CharacterID
	WHERE GI.DNGirlCharacterID = @inbDNGirlCharacterID AND GI.FanFlag = 1
	ORDER BY GI.Intimacy DESC;
END TRY
BEGIN CATCH
	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
