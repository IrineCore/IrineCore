USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[BoardComments](
	[CommentsSeq] [int] IDENTITY(1,1) NOT NULL,
	[BoardSeq] [int] NOT NULL,
	[UserIndex] [int] NOT NULL,
	[UserID] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[Comments] [nvarchar](max) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[WriteDate] [datetime] NOT NULL,
	[UpdateDate] [smalldatetime] NULL,
	[DeleteDate] [smalldatetime] NULL,
	[DeleteFlag] [tinyint] NOT NULL
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
CREATE CLUSTERED INDEX [IX_BoardComments] ON [dbo].[BoardComments]
(
	[BoardSeq] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[BoardComments] ADD CONSTRAINT [PK_BoardComments] PRIMARY KEY NONCLUSTERED
(
	[CommentsSeq] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[BoardComments] WITH CHECK ADD CONSTRAINT [FK_BoardComments_BoardMain] FOREIGN KEY([BoardSeq])
REFERENCES [dbo].[BoardMain] ([BoardSeq])
GO
ALTER TABLE [dbo].[BoardComments] CHECK CONSTRAINT [FK_BoardComments_BoardMain]
GO
