USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DarkLairRankingTemp](
	[DarkLairRankingID] [bigint] NOT NULL,
	[MapID] [int] NOT NULL,
	[PartyUserCount] [tinyint] NOT NULL,
	[PlayRound] [smallint] NOT NULL,
	[PlayTime] [int] NOT NULL,
	[PartyName] [nvarchar](100) NOT NULL,
	[RegisterDate] [datetime] NOT NULL,
	[BestFlag] [bit] NOT NULL,
	[BlockFlag] [int] NOT NULL
)
GO
