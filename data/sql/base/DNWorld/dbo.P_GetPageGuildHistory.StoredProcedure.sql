USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageGuildHistory]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-27
description : ?? ??? ?? ??? ?????? ?????.
result set : GuildHistoryCode/tinyint, CharacterID/bigint, CharacterName/nvarchar, CharacterLevel/tinyint, GuildMemberRoleCode/tinyint, ItemID/int, ItemLevel/tinyint, GuildLevel/tinyint, GuildDepotSize/smallint, RegisterDate/datetime2, GuildNameBefore/nvarchar, GuildNameAfter/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103244 = ?? ??? ????.
history : 
7 = @inyGuildHistoryCode ?? NULL? ? GuildHistoryCode ? 200 ??? ?? ????? ?? at 2010-10-11
8 = GuildHistoryCode ? ? ?? ?? ?? at 2010-10-20
**/

CREATE PROCEDURE [dbo].[P_GetPageGuildHistory]
	  @intStartNumber int --// ?? ??.
	, @intEndNumber int --// ? ??.
	, @inyDirection tinyint --// ?? ??. 1=????, 2=????
	, @intGuildID int --// ?? ID
	, @inyGuildHistoryCode tinyint = NULL --// NULL ??? ?? (?, 200??? ??). 1=?? ??, 2=?? ?? ??, 3=??? ??, 4=??? ??, 5=??? ??, 6=??? ?? ??, 7=??? ?? ?, 8=??? ? ?? ??, 9=?? ??, 10=?? ?? ?? ??, 11=?? ?? ??, 254=?? ??? ?? (??), 255=?? ??? ?? (??)
	, @intRecordCount int = 0 OUTPUT --// ? ?? ?? ? (@intStartNumber ?? 1? ?? ?????.)
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
	IF @intStartNumber = 1
		SELECT @intRecordCount = RecordCount
		FROM dbo.VIX_GuildHistoryCount WITH (NOEXPAND, NOLOCK)
		WHERE GuildID = @intGuildID;

	IF @inyDirection = 1
		WITH PagingCTE AS (
			SELECT ROW_NUMBER() OVER(ORDER BY GuildID, GuildHistoryID DESC) AS RowNumber
				, GuildHistoryCode, CharacterID, CharacterLevel, GuildMemberRoleCode, ItemID, ItemLevel, GuildLevel, GuildDepotSize, RegisterDate, GuildNameBefore, GuildNameAfter
			FROM dbo.GuildHistory WITH (NOLOCK)
			WHERE GuildID = @intGuildID AND GuildHistoryCode = COALESCE(@inyGuildHistoryCode, GuildHistoryCode) AND GuildHistoryCode < 200
		)
		SELECT PagingCTE.GuildHistoryCode, PagingCTE.CharacterID, CH.CharacterName, PagingCTE.CharacterLevel, PagingCTE.GuildMemberRoleCode
			, PagingCTE.ItemID, PagingCTE.ItemLevel, PagingCTE.GuildLevel, PagingCTE.GuildDepotSize, PagingCTE.RegisterDate, PagingCTE.GuildNameBefore, PagingCTE.GuildNameAfter
		FROM PagingCTE
			LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON PagingCTE.CharacterID = CH.CharacterID
		WHERE PagingCTE.RowNumber BETWEEN @intStartNumber AND @intEndNumber
		ORDER BY PagingCTE.RowNumber
		OPTION (MAXDOP 1);

	ELSE
		WITH PagingCTE AS (
			SELECT ROW_NUMBER() OVER(ORDER BY GuildID, GuildHistoryID) AS RowNumber
				, GuildHistoryCode, CharacterID, CharacterLevel, GuildMemberRoleCode, ItemID, ItemLevel, GuildLevel, GuildDepotSize, RegisterDate, GuildNameBefore, GuildNameAfter
			FROM dbo.GuildHistory WITH (NOLOCK)
			WHERE GuildID = @intGuildID AND GuildHistoryCode = COALESCE(@inyGuildHistoryCode, GuildHistoryCode) AND GuildHistoryCode < 200
		)
		SELECT PagingCTE.GuildHistoryCode, PagingCTE.CharacterID, CH.CharacterName, PagingCTE.CharacterLevel, PagingCTE.GuildMemberRoleCode
			, PagingCTE.ItemID, PagingCTE.ItemLevel, PagingCTE.GuildLevel, PagingCTE.GuildDepotSize, PagingCTE.RegisterDate, PagingCTE.GuildNameBefore, PagingCTE.GuildNameAfter
		FROM PagingCTE
			LEFT OUTER JOIN dbo.Characters CH WITH (NOLOCK) ON PagingCTE.CharacterID = CH.CharacterID
		WHERE PagingCTE.RowNumber BETWEEN @intStartNumber AND @intEndNumber
		ORDER BY PagingCTE.RowNumber
		OPTION (MAXDOP 1);

	/**_## ??? ???? ?? ???? ???, 103244? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103244;
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
