USE [dnworld]
GO
/****** Object:  Table [dbo].[GambleIDs]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GambleIDs](
	[GambleID] [bigint] IDENTITY(1,1) NOT NULL,
	[GambleFeeCode] [tinyint] NOT NULL,
	[EntranceFee] [int] NOT NULL,
	[GambelStartDate] [datetime2](0) NOT NULL,
	[GambelEndDate] [datetime2](0) NULL,
	[WinCharacterID] [bigint] NULL,
	[SemiWinCharacterID] [bigint] NULL,
	[WinPrize] [int] NULL,
	[SemiWinPrize] [int] NULL,
	[RewardFlag] [bit] NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_GambleIDs_GambelStartDate]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_GambleIDs_GambelStartDate] ON [dbo].[GambleIDs]
(
	[GambelStartDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_GambleIDs]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[GambleIDs] ADD  CONSTRAINT [PK_GambleIDs] PRIMARY KEY NONCLUSTERED 
(
	[GambleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[GambleIDs] ADD  CONSTRAINT [DF_GambleIDs_RewardFlag]  DEFAULT ((0)) FOR [RewardFlag]
GO
