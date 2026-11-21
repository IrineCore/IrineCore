USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[RestraintReasons](
	[RestraintReasonID] [int] IDENTITY(1,1) NOT NULL,
	[RestraintReasonNode] [hierarchyid] NOT NULL,
	[RestraintReasonLevel] AS ([RestraintReasonNode].[GetLevel]()),
	[RestraintReasonName] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[AdminID] [int] NOT NULL,
	[RegisterDate] [smalldatetime] NOT NULL,
	[UsedFlag] [bit] NOT NULL
) ON [PRIMARY]
GO
CREATE UNIQUE CLUSTERED INDEX [IX_UC_RestraintReasons_RestraintReasonNode] ON [dbo].[RestraintReasons]
(
	[RestraintReasonNode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[RestraintReasons] ADD CONSTRAINT [PK_RestraintReasons] PRIMARY KEY NONCLUSTERED
(
	[RestraintReasonID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_RestraintReasons_RestraintReasonLevel_RestraintReasonNode] ON [dbo].[RestraintReasons]
(
	[RestraintReasonLevel] ASC,
	[RestraintReasonNode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[RestraintReasons] ADD CONSTRAINT [DF_RestraintReasons_UsedFlag] DEFAULT ((0)) FOR [UsedFlag]
GO
