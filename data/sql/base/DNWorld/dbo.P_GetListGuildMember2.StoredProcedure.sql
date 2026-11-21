USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildMember2]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-27
description : ??? ??? ?????.
result set : AccountID/int, AccountName/nvarchar, CharacterID/bigint, CharacterName/nvarchar, JobCode/tinyint, CharacterLevel/tinyint, LastLogoutDate/datetime2, GuildMemberRoleCode/tinyint, SelfIntroduction/nvarchar, TotalGuildPoint/int, UsedGuildPoint/int, RegisterDate/datetime2, DailyTakeItemCount/int, DailyTakeCoinAmount/int, LastGuildDepotUseDate/smalldatetime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103238 = ??? ???? ????.
history :
3 = ?? ?? ?? (DailyTakeItemCount, DailyTakeCoinAmount, LastGuildDepotUseDate) at 2010-10-19
6 = UNION ALL ? UNION ?? ?? (????? ????? ???? ???? ????? ??? ?? ??) by ??? at 2011-11-14
7~8 = ?? ??? ?? ??? ?? ??? ?? LEFT OUTER JOIN ???? ?? by ??? at 2011-11-14
9 = DormantCharacterStatus ??? ?? ?? by ??? at 2011-11-30
10~12 = ?? ?? by ??? at 2013-01-02
**/

CREATE PROCEDURE [dbo].[P_GetListGuildMember2]
	  @intGuildID int = NULL --// ?? ID
	, @nvcGuildName nvarchar(50) = NULL --// ?? ??
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
	IF @intGuildID > 0
		SELECT CH.AccountID, CH.AccountName, CH.CharacterID, CH.CharacterName, CS.JobCode, CS.CharacterLevel, CS.LastLogoutDate
			, GM.GuildMemberRoleCode, GM.SelfIntroduction, GM.TotalGuildPoint, GM.UsedGuildPoint, GM.RegisterDate, GM.DailyTakeItemCount, GM.DailyTakeCoinAmount, GM.LastGuildDepotUseDate
		FROM dbo.GuildMembers GM WITH (NOLOCK, INDEX(PK_GuildMembers))
			INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters)) ON GM.CharacterID = CH.CharacterID
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK, INDEX(PK_CharacterStatus)) ON CH.CharacterID = CS.CharacterID
		WHERE GM.GuildID = @intGuildID
		OPTION(FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
	ELSE
		SELECT CH.AccountID, CH.AccountName, CH.CharacterID, CH.CharacterName, CS.JobCode, CS.CharacterLevel, CS.LastLogoutDate
			, GM.GuildMemberRoleCode, GM.SelfIntroduction, GM.TotalGuildPoint, GM.UsedGuildPoint, GM.RegisterDate
		FROM dbo.Guilds G WITH (NOLOCK, INDEX(IX_UNF_Guilds_GuildName))
			INNER JOIN dbo.GuildMembers GM WITH (NOLOCK, INDEX(PK_GuildMembers)) ON G.GuildID = GM.GuildID
			INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK, INDEX(PK_CharacterStatus)) ON CS.CharacterID = GM.CharacterID			
			INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(PK_Characters)) ON CH.CharacterID = GM.CharacterID
		WHERE G.GuildName = @nvcGuildName AND G.CloseFlag = CAST(0 AS bit)
		OPTION(FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103238;
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
