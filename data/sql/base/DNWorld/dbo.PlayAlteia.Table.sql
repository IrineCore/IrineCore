USE [dnworld]
GO
/****** Object:  Table [dbo].[PlayAlteia]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PlayAlteia](
	[CharacterID] [bigint] NOT NULL,
	[AcquireGoldKeyCount] [int] NOT NULL,
	[PlayTime] [int] NOT NULL,
	[GoldKeyRegisterDate] [datetime2](0) NOT NULL,
	[PlayTimeRegisterDate] [datetime2](0) NOT NULL,
	[DailyPlayCount] [tinyint] NOT NULL,
	[WeeklyPlayCount] [tinyint] NOT NULL,
	[LastPlayingDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_PlayAlteia] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_PlayAlteia_AcquireGoldKeyCount_GoldKeyRegisterDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PlayAlteia_AcquireGoldKeyCount_GoldKeyRegisterDate] ON [dbo].[PlayAlteia]
(
	[AcquireGoldKeyCount] DESC,
	[GoldKeyRegisterDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_PlayAlteia_PlayTime_PlayTimeRegisterDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PlayAlteia_PlayTime_PlayTimeRegisterDate] ON [dbo].[PlayAlteia]
(
	[PlayTime] ASC,
	[PlayTimeRegisterDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
