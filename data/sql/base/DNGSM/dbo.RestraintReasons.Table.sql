USE [dngsm]
GO
/****** Object:  Table [dbo].[RestraintReasons]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[RestraintReasons](
	[RestraintReasonID] [int] IDENTITY(1,1) NOT NULL,
	[RestraintReasonNode] [hierarchyid] NOT NULL,
	[RestraintReasonLevel]  AS ([RestraintReasonNode].[GetLevel]()),
	[RestraintReasonName] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[AdminID] [int] NOT NULL,
	[RegisterDate] [smalldatetime] NOT NULL,
	[UsedFlag] [bit] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UC_RestraintReasons_RestraintReasonNode]    Script Date: 11/14/2026 8:47:03 AM ******/
CREATE UNIQUE CLUSTERED INDEX [IX_UC_RestraintReasons_RestraintReasonNode] ON [dbo].[RestraintReasons]
(
	[RestraintReasonNode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [PK_RestraintReasons]    Script Date: 11/14/2026 8:47:09 AM ******/
ALTER TABLE [dbo].[RestraintReasons] ADD  CONSTRAINT [PK_RestraintReasons] PRIMARY KEY NONCLUSTERED 
(
	[RestraintReasonID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
SET ARITHABORT ON
SET CONCAT_NULL_YIELDS_NULL ON
SET QUOTED_IDENTIFIER ON
SET ANSI_NULLS ON
SET ANSI_PADDING ON
SET ANSI_WARNINGS ON
SET NUMERIC_ROUNDABORT OFF
GO
/****** Object:  Index [IX_UN_RestraintReasons_RestraintReasonLevel_RestraintReasonNode]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_RestraintReasons_RestraintReasonLevel_RestraintReasonNode] ON [dbo].[RestraintReasons]
(
	[RestraintReasonLevel] ASC,
	[RestraintReasonNode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[RestraintReasons] ADD  CONSTRAINT [DF_RestraintReasons_UsedFlag]  DEFAULT ((0)) FOR [UsedFlag]
GO
