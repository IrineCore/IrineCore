USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountPVPRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-08-03
description : PVP ?? ???? ?? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetCountPVPRanking]	  
	  @inyCharacterClassCode tinyint = 0 --// ??? ?????
	, @inyCharacterSubClassCode tinyint = 0 --// 2? ?? ??	
	, @intGuildID int = 0 --// ????ID
	, @nvcGuildName nvarchar(50) = N'' --// ?????
	, @inbTotalCount bigint OUTPUT --// ??? ???? ?? ??? ?? ?????.	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int,
		@nvcStmt nvarchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [PVPRanking] ???? SELECT???.*/
	SET @nvcStmt = N'
SELECT @inbTotalCount = COUNT(*)
FROM dbo.PVPRanking WITH (NOLOCK)	
WHERE CharacterID IS NOT NULL' +
		CASE WHEN @inyCharacterClassCode > 0 THEN N' AND CharacterClassCode = @inyCharacterClassCode' ELSE N'' END +
		CASE WHEN @inyCharacterSubClassCode > 0  THEN N' AND CharacterSubClassCode = @inyCharacterSubClassCode' ELSE N'' END +
		CASE WHEN @intGuildID > 0  THEN N' AND GuildID = @intGuildID' ELSE N'' END +
		CASE WHEN @nvcGuildName > N'' THEN N' AND GuildName LIKE @nvcGuildName' ELSE N'' END + N'
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @inyCharacterClassCode tinyint
			, @inyCharacterSubClassCode tinyint
			, @intGuildID int
			, @nvcGuildName nvarchar(50)			
			, @inbTotalCount bigint OUTPUT
		'
		, @inyCharacterClassCode = @inyCharacterClassCode
		, @inyCharacterSubClassCode = @inyCharacterSubClassCode
		, @intGuildID = @intGuildID
		, @nvcGuildName = @nvcGuildName		
		, @inbTotalCount = @inbTotalCount OUTPUT;
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
