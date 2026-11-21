USE [dnworld]
GO
/****** Object:  Table [dbo].[Parties]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Parties](
	[PartyID] [bigint] IDENTITY(1,1) NOT NULL,
	[ServerID] [int] NOT NULL,
	[WorldID] [int] NOT NULL,
	[RoomID] [int] NOT NULL,
	[ServerType] [tinyint] NOT NULL,
	[ServerCommonID] [int] NOT NULL,
	[PartyCode] [tinyint] NOT NULL,
	[PartyName] [nvarchar](40) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[PartyMaxCount] [smallint] NOT NULL,
	[GoalMapID] [int] NOT NULL,
	[DifficultyCode] [tinyint] NOT NULL,
	[PartyOptionFlagBitmap] [int] NOT NULL,
	[PassWordValue] [int] NULL,
	[MinLevel] [tinyint] NOT NULL,
	[DefaultDistributionCode] [tinyint] NOT NULL,
	[RandomDistributionCode] [tinyint] NOT NULL,
	[UpkeepCount] [int] NOT NULL,
 CONSTRAINT [PK_Parties] PRIMARY KEY CLUSTERED 
(
	[PartyID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_Parties_ServerID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Parties_ServerID] ON [dbo].[Parties]
(
	[ServerID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
