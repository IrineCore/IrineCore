USE [dnworld]
GO
/****** Object:  Table [dbo].[NestGateStartIDs]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NestGateStartIDs](
	[NestGateStartID] [bigint] IDENTITY(1,1) NOT NULL,
	[RoomID] [bigint] NOT NULL,
	[StageStartID] [bigint] NULL,
	[MapID] [int] NOT NULL,
	[NestRound] [tinyint] NOT NULL,
	[NestGateStartDate] [datetime2](0) NOT NULL,
	[NestGateEndDate] [datetime2](0) NULL,
	[StartPartySize] [int] NOT NULL,
	[EndPartySize] [int] NULL,
 CONSTRAINT [PK_NestGateStartIDs] PRIMARY KEY CLUSTERED 
(
	[NestGateStartID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_NestGateStartIDs_NestGateStartDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_NestGateStartIDs_NestGateStartDate] ON [dbo].[NestGateStartIDs]
(
	[NestGateStartDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_NestGateStartIDs_RoomID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_NestGateStartIDs_RoomID] ON [dbo].[NestGateStartIDs]
(
	[RoomID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
