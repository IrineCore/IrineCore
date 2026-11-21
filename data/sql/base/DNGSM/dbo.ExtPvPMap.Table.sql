USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtPvPMap]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtPvPMap](
	[id] [int] NULL,
	[_LadderType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[MapTableID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[MapNameUIstring] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTpye_UIStingPvP] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTpye_UIStingCOOP] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTableID_1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTableID_2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTableID_3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTableID_4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTableID_5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTableID_6] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTableID_7] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTableID_8] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTableID_9] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[GameModeTableID_10] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[NumOfPlayersOption1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[NumOfPlayersOption2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[NumOfPlayersOption3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[NumOfPlayersOption4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[NumOfPlayersOption5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[DefaultNumOfPlayers] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[Allow_Breakin_PlayingGame] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[ItemUsageType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[AllowItemDrop] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[IsGuildBattleGround] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[ReleaseShow] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[MapImage] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[UIStringMapNameUIstring] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtPvPMap_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtPvPMap_id] ON [dbo].[ExtPvPMap]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
