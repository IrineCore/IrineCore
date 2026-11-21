USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuild]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-18
description : ?? ??? ?????.
result set : 
	version 8 = GuildID, GuildName, GuildLevel, GuildCoin, GuildDepotSize, NoticeContent, RemainGuildPoint, TotalGuildPoint, RegisterDate, CloseFlag, CloseDate
	version 9 = GuildID, GuildName, GuildLevel, GuildCoin, GuildDepotSize, NoticeContent, RegisterDate, CloseFlag, CloseDate, TotalGuildEXP, GuildMaxMemberCount
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103200 = ??? ???? ??? ????.
history :
8 = input param. @intVersion ?? at 2010-12-09
9 = ?? ?? at 2010-12-09
10 = ?? ???? ?? ?? by ??? at 2011-12-13
11 = ???? ?? ??? ??? ?? ?? by ??? at 2011-12-29
**/

CREATE PROCEDURE [dbo].[P_GetListGuild]
	  @nvcName nvarchar(20) = NULL --// ?? ??
	, @insMinLevel smallint = NULL --// ?? ?? ??
	, @insMaxLevel smallint = NULL --// ?? ?? ??
	, @sdtStartDate smalldatetime = NULL --// ?? ?? ?? (?? ?? ?? ??)
	, @sdtEndDate smalldatetime = NULL --// ?? ? ?? (?? ?? ?? ??)
	, @inbMemberCharacterID bigint = NULL --// ?? ?? ??? ID
	, @intVersion int = 8 --// ?? ?? ?? 8? ?????.
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int,
	@sdtCloseDate smalldatetime = '1900-01-01 00:00:00',
	@bitCloseFlag bit = 0;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @intColumnCount = 7;

	SET @inbNullBitmap = 
		CASE WHEN @nvcName IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @insMinLevel IS NOT NULL THEN POWER(2, 1) ELSE 0 END + 
		CASE WHEN @insMaxLevel IS NOT NULL THEN POWER(2, 2) ELSE 0 END + 
		CASE WHEN @sdtStartDate IS NOT NULL THEN POWER(2, 3) ELSE 0 END + 
		CASE WHEN @sdtEndDate IS NOT NULL THEN POWER(2, 4) ELSE 0 END + 
		CASE WHEN @inbMemberCharacterID IS NOT NULL THEN POWER(2, 5) ELSE 0 END +
		CASE WHEN @sdtCloseDate IS NOT NULL THEN POWER(2, 6) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	IF @intVersion = 8
		SET @nvcStmt = N'
SELECT G.GuildID, G.GuildName, G.GuildLevel, G.GuildCoin, G.GuildDepotSize, G.NoticeContent, G.RemainGuildPoint, G.TotalGuildPoint, G.RegisterDate, G.CloseFlag, G.CloseDate
FROM dbo.Guilds G WITH (NOLOCK)
'
		+ CASE WHEN @inbMemberCharacterID > 0 THEN N'	INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON G.GuildID = GM.GuildID' ELSE N'' END
		+ dbo.FN_DynamicWhereClause2(N'G.GuildName', N'@nvcName', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'G.GuildLevel', N'@insMinLevel', @inbNullBitmap, 2, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'G.GuildLevel', N'@insMaxLevel', @inbNullBitmap, 3, @intColumnCount, '<=')
		+ dbo.FN_DynamicWhereClause2(N'G.RegisterDate', N'@sdtStartDate', @inbNullBitmap, 4, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'G.RegisterDate', N'@sdtEndDate', @inbNullBitmap, 5, @intColumnCount, '<=')
		+ dbo.FN_DynamicWhereClause2(N'GM.CharacterID', N'@inbMemberCharacterID', @inbNullBitmap, 6, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'G.CloseFlag', N'@bitCloseFlag', @inbNullBitmap, 7, @intColumnCount, '=')
 + N'
ORDER BY G.GuildID';

	ELSE IF @intVersion = 9
		SET @nvcStmt = N'
SELECT G.GuildID, G.GuildName, G.GuildLevel, G.GuildCoin, G.GuildDepotSize, G.NoticeContent, G.RegisterDate, G.CloseFlag, G.CloseDate, G.TotalGuildEXP, G.GuildMaxMemberCount
FROM dbo.Guilds G WITH (NOLOCK)
'
		+ CASE WHEN @inbMemberCharacterID > 0 THEN N'	INNER JOIN dbo.GuildMembers GM WITH (NOLOCK) ON G.GuildID = GM.GuildID' ELSE N'' END
		+ dbo.FN_DynamicWhereClause2(N'G.GuildName', N'@nvcName', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'G.GuildLevel', N'@insMinLevel', @inbNullBitmap, 2, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'G.GuildLevel', N'@insMaxLevel', @inbNullBitmap, 3, @intColumnCount, '<=')
		+ dbo.FN_DynamicWhereClause2(N'G.RegisterDate', N'@sdtStartDate', @inbNullBitmap, 4, @intColumnCount, '>=')
		+ dbo.FN_DynamicWhereClause2(N'G.RegisterDate', N'@sdtEndDate', @inbNullBitmap, 5, @intColumnCount, '<=')
		+ dbo.FN_DynamicWhereClause2(N'GM.CharacterID', N'@inbMemberCharacterID', @inbNullBitmap, 6, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'G.CloseFlag', N'@bitCloseFlag', @inbNullBitmap, 7, @intColumnCount, '=')
 + N'
ORDER BY G.GuildID';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @nvcName nvarchar(20)
			, @insMinLevel smallint
			, @insMaxLevel smallint
			, @sdtStartDate smalldatetime
			, @sdtEndDate smalldatetime
			, @inbMemberCharacterID bigint
			, @sdtCloseDate smalldatetime
			, @bitCloseFlag bit
		'
		, 	  @nvcName = @nvcName
			, @insMinLevel = @insMinLevel
			, @insMaxLevel = @insMaxLevel
			, @sdtStartDate = @sdtStartDate
			, @sdtEndDate = @sdtEndDate
			, @inbMemberCharacterID = @inbMemberCharacterID
			, @sdtCloseDate = @sdtCloseDate
			, @bitCloseFlag = @bitCloseFlag;
	/**_## ??? ???? ?? ???? ???, 103200? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103200;
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
